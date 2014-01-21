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

type Demand struct {
	Steering float32
	Throttle float32
}

// The merged status of all of the inputs and sensors.
// Used as input to the controller.
type Status struct {
	Allowed bool

	GPS GPS
	Input Input
}

type Driver struct {
	Status     Status

	GPS        *gps.Link
	Link       *link.Link
	Controller Controller

	Switch *button.Button

	recorder Recorder

	rmcSeen    Timeout
	ggaSeen    Timeout
	inputSeen  Timeout
	inputScale float32

	refOk          bool
	latitudeRef    float64
	longitudeRef   float64
	latitudeScale  float64
	longitudeScale float64
}

func headingToRad(heading float32) float32 {
	if heading > 180 {
		return (heading - 360) * (math.Pi / 360)
	} else {
		return heading * (math.Pi / 360)
	}
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
	d.Status.GPS.Time = msg.Time
	d.Status.GPS.Latitude = msg.Latitude
	d.Status.GPS.Longitude = msg.Longitude
	d.Status.GPS.Speed = msg.Speed * KnotsToMs
	d.Status.GPS.Track = headingToRad(msg.Track)

	if !d.refOk {
		d.updateRef()
	}

	glog.V(2).Infof("sentence: %T %+v\n", msg, msg)
}

func (d *Driver) gga(msg *gps.GGA) {
	d.ggaSeen.Start(time.Second)
	d.Status.GPS.NumSatellites = msg.NumSatellites

	glog.V(2).Infof("sentence: %T %+v\n", msg, msg)
}

func (d *Driver) sentence(sentence gps.Sentence) {
	switch sentence.(type) {
	case *gps.RMC:
		d.rmc(sentence.(*gps.RMC))
	case *gps.GGA:
		d.gga(sentence.(*gps.GGA))
	default:
		glog.V(2).Infof("sentence: %T", sentence)
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

	fcpu := 8000000 + int(msg.Reference)*(1000000/64)
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
	glog.V(2).Infof("frame: %T %+v\n", msg, msg)
}

func (d *Driver) heartbeat(msg *link.Heartbeat) {
	glog.V(2).Infof("frame: %T %+v\n", msg, msg)
}

func (d *Driver) frame(frame link.Frame) {
	switch frame.(type) {
	case *link.Input:
		d.input(frame.(*link.Input))
	case *link.Heartbeat:
		d.heartbeat(frame.(*link.Heartbeat))
	default:
		glog.V(2).Infof("frame: %T %+v\n", frame, frame)
	}
}

func (d *Driver) button(event *button.Event) {
	d.recorder.Switch(event)
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
	v = mathex.Clipf(v, -0.95, 0.95)
	return int8(v / d.inputScale)
}

func (d *Driver) step() {
	demand := d.Controller.Step(&d.Status)
	msg := &link.Demand{Code: 'd', Flags: 1}

	for i := range msg.Channels {
		msg.Channels[i] = Missing
	}
	msg.Channels[SteeringChannel] = d.toDemand(demand.Steering)
	msg.Channels[ThrottleChannel] = d.toDemand(demand.Throttle)

	d.Link.Send(msg)
	glog.V(2).Infof("demand: %T %+v\n", demand, demand)
	glog.V(2).Infof("sent: %T %+v\n", msg, msg)
}

func (d *Driver) Run() {
	tick := time.Tick(time.Millisecond * (Dt*1000))
	heartbeat := time.Tick(time.Second*5)

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
			glog.V(1).Infoln("heartbeat")
		}
	}
}
