package rover

import (
	"math"
	"time"
)

import (
	"juju.net.nz/nppilot/rover/gps"
	"juju.net.nz/nppilot/rover/link"
	"juju.net.nz/nppilot/rover/button"
	"juju.net.nz/mathex"
	"github.com/golang/glog"
)

const (
	Dt              = 0.05
	KnotsToMs       = 0.51444
	InputMax        = 70
	SteeringChannel = 0
	ThrottleChannel = 2
	SwitchChannel   = 4
	DialChannel     = 5
	Missing         = -128
	Prescaler       = 64
)

type GPS struct {
	Ok bool

	// The following are only valid of GpsOk is set.

	// Number of satellites in the lock.
	NumSatellites int
	// Time of last fix in seconds since midnight.
	Time float64
	// Latitude in decimal degrees.
	Latitude float64
	// Longitude in decimal degrees.
	Longitude float64
	// Speed in m/s.
	Speed float32
	// Heading in radians from -PI to PI.
	Track float32
	// Metres north from the reference.
	North float64
	// Metres east from the reference.
	East float64
}

type Input struct {
	Ok bool

	// Current switch position from 0..2.  -1 means unknown.
	Switch int

	Steering float32
	Throttle float32
	Dial     float32
}

type IMU struct {
	Ok bool

	RateOfTurn float32
}

type Demand struct {
	Steering float32
	Throttle float32
}

// The merged status of all of the inputs and sensors.
// Used as input to the controller.
type Status struct {
	Allowed bool

	GPS GPS
	IMU IMU
	Input Input

	Recorder *Recorder
}

type Driver struct {
	Status     Status

	GPS        *gps.Link
	Link       *link.Link
	Controller Controller

	Switch *button.Button

	SteeringOffset float32

	rmcSeen    Timeout
	ggaSeen    Timeout
	inputSeen  Timeout
	inputScale float32

	refOk          bool
	latitudeRef    float64
	longitudeRef   float64
	latitudeScale  float64
	longitudeScale float64

	gpsWatchdog *Watchdog
	gpsLockWatchdog *Watchdog
	heartbeatWatchdog *Watchdog
	pongWatchdog *Watchdog
	inputWatchdog *Watchdog
	imuWatchdog *Watchdog
}

func New() *Driver {
	d := &Driver{
		gpsWatchdog: &Watchdog{Name: "gps"},
		gpsLockWatchdog: &Watchdog{Name: "gpsLock"},
		heartbeatWatchdog: &Watchdog{Name: "heartbeat"},
		pongWatchdog: &Watchdog{Name: "pong"},
		inputWatchdog: &Watchdog{Name: "input"},
		imuWatchdog: &Watchdog{Name: "imu"},
	}

	return d
}

func Always(source string, arg interface{}) {
	glog.V(1).Infof("%s: %T %+v", source, arg, arg)
}

func Info(source string, arg interface{}) {
	glog.V(2).Infof("%s: %T %+v", source, arg, arg)
}

func headingToRad(heading float32) float32 {
	heading *= math.Pi/180

	if heading > math.Pi {
		heading -= 2*math.Pi
	}
	return heading
}

func (s *Driver) updateRef() {
	s.latitudeRef = s.Status.GPS.Latitude
	s.longitudeRef = s.Status.GPS.Longitude

	s.latitudeScale = LatitudeLen(s.Status.GPS.Latitude)
	s.longitudeScale = LongitudeLen(s.Status.GPS.Longitude)
	s.refOk = true
}

func (d *Driver) rmc(msg *gps.RMC) {
	d.rmcSeen.Start(time.Second)
	d.gpsWatchdog.Feed()

	d.Status.GPS.Time = msg.Time
	d.Status.GPS.Latitude = msg.Latitude
	d.Status.GPS.Longitude = msg.Longitude
	d.Status.GPS.Speed = msg.Speed * KnotsToMs
	d.Status.GPS.Track = headingToRad(msg.Track)

	if !d.refOk {
		d.updateRef()
	}

	d.Controller.GPS(&d.Status)

	Info("sentence", msg)
	Info("rmc", d.Status.GPS)
}

func (d *Driver) gga(msg *gps.GGA) {
	d.ggaSeen.Start(time.Second)
	d.Status.GPS.NumSatellites = msg.NumSatellites

	Info("sentence", msg)
}

func (d *Driver) vtg(msg *gps.VTG) {
	d.gpsWatchdog.Feed()
	Info("sentence", msg)
}

func (d *Driver) sentence(sentence gps.Sentence) {
	switch sentence.(type) {
	case *gps.RMC:
		d.rmc(sentence.(*gps.RMC))
	case *gps.GGA:
		d.gga(sentence.(*gps.GGA))
	case *gps.VTG:
		d.vtg(sentence.(*gps.VTG))
	default:
		Info("sentence", sentence)
	}
}

func scaleInput(input int8, reference float32) float32 {
	switch {
	case reference <= 0:
		return Missing
	case input <= Missing:
		return Missing
	default:
		return float32(input) * reference
	}
}

func (d *Driver) input(msg *link.Input) {
	d.inputSeen.Start(time.Second)
	d.inputWatchdog.Feed()

	fcpu := 8000000
	if msg.Reference == 2 {
		fcpu = 12000000
	}
	d.inputScale = Prescaler * 1000 * 2 / float32(fcpu)

	d.Status.Input.Steering = scaleInput(msg.Channels[SteeringChannel], d.inputScale)
	d.Status.Input.Throttle = scaleInput(msg.Channels[ThrottleChannel], d.inputScale)
	d.Status.Input.Dial = scaleInput(msg.Channels[DialChannel], d.inputScale)

	switchIn := scaleInput(msg.Channels[SwitchChannel], d.inputScale)
	switch {
	case switchIn < -0.5:
		d.Status.Input.Switch = 0
	case switchIn < 0.5:
		d.Status.Input.Switch = 1
	default:
		d.Status.Input.Switch = 2
	}
	d.Switch.Level <- d.Status.Input.Switch
	Info("input", d.Status.Input)
}

func (d *Driver) heartbeat(msg *link.Heartbeat) {
	d.heartbeatWatchdog.Feed()
}

func (d *Driver) pong(msg *link.Pong) {
	d.pongWatchdog.Feed()
}

func (d *Driver) imu(msg *link.IMU) {
	d.Status.IMU.RateOfTurn = float32(-msg.Gyros[2]) * (250.0/32768 * math.Pi/180)
	Info("imu", d.Status.IMU)
	d.imuWatchdog.Feed()
}

func (d *Driver) frame(frame link.Frame) {
	switch frame.(type) {
	case *link.Input:
		d.input(frame.(*link.Input))
	case *link.Heartbeat:
		d.heartbeat(frame.(*link.Heartbeat))
	case *link.Pong:
		d.pong(frame.(*link.Pong))
	case *link.IMU:
		d.imu(frame.(*link.IMU))
	}
	Info("frame", frame)
}

func (d *Driver) ping() {
	glog.V(2).Infof("ping")
	d.Link.Send(&link.Request{Code: 'R', Requested: 'p'})
}

func (d *Driver) button(event *button.Event) {
	next := d.Status.Recorder.Switch(event)
	if next != None {
		d.Controller.Event(next)
	}
}

func (d *Driver) checkAll() {
	d.Status.GPS.Ok = d.rmcSeen.Running() &&
		d.ggaSeen.Running() &&
		d.Status.GPS.NumSatellites >= 6

	if !d.ggaSeen.Running() {
		d.Status.GPS.NumSatellites = -1
	}
	d.Status.Input.Ok = d.inputSeen.Running()
	if !d.Status.Input.Ok {
		d.Status.Input.Switch = -1
	}
	d.Status.Allowed = true
}

func (d *Driver) toDemand(v float32) int8 {
	switch {
	case d.inputScale <= 0:
		return Missing
	case v < -10:
		return Missing
	}
	v = mathex.Clipf(v, -1, 1)
	return int8(v / d.inputScale)
}

func (d *Driver) step() {
	d.Status.Recorder.GPS(&d.Status.GPS)

	demand := d.Controller.Step(&d.Status)
	msg := &link.Demand{Code: 'd', Flags: 1}

	for i := range msg.Channels {
		msg.Channels[i] = Missing
	}
	msg.Channels[SteeringChannel] = d.toDemand(demand.Steering + d.SteeringOffset)
	msg.Channels[ThrottleChannel] = d.toDemand(demand.Throttle)

	d.Link.Send(msg)
	Info("demand", demand)
	Info("sent", msg)
}

func (d *Driver) checkWatchdogs() {
	d.gpsWatchdog.Check()
	d.gpsLockWatchdog.Check()
	d.heartbeatWatchdog.Check()
	d.pongWatchdog.Check()
	d.inputWatchdog.Check()
	d.imuWatchdog.Check()
}

func (d *Driver) localHeartbeat() {
	d.ping()
	d.checkWatchdogs()
	Always("link", d.Link.Stats)
	Always("gps", d.GPS.Stats)
	glog.V(1).Infof("heartbeat")
	glog.Flush()
}

func (d *Driver) Run() {
	tick := time.Tick(time.Millisecond * (Dt*1000))
	heartbeat := time.Tick(time.Second*1)

	for {
		select {
		case sentence := <-d.GPS.Sentences:
			d.sentence(sentence)
		case frame := <-d.Link.Frames:
			d.frame(frame)
		case event := <-d.Switch.Event:
			d.button(event)
		case <-tick:
			d.checkAll()
			d.step()
		case <-heartbeat:
			d.localHeartbeat()
		}
	}
}
