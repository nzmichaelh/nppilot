package rover

import (
	"fmt"
	"log"
	"math"
	"time"
)

import (
	"juju.net.nz/nppilot/rover/gps"
	"juju.net.nz/nppilot/rover/link"
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

// The merged status of all of the inputs and sensors.
// Used as input to the controller.
type Status struct {
	GpsOk   bool
	InputOk bool
	Allowed bool

	// Current switch position from 0..2.  -1 means unknown.
	Switch int

	SteeringIn float32
	ThrottleIn float32
	DialIn     float32

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
	Speed float64
	// Heading in radians from -PI to PI.
	Track float64
	// Metres north from the reference.
	North float64
	// Metres east from the reference.
	East float64
}

type Supervisor struct {
	GPS        *gps.Link
	Link       *link.Link
	Controller *SpeedController
	Status     Status

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

func headingToRad(heading float64) float64 {
	if heading > 180 {
		return (heading - 360) * (math.Pi / 360)
	} else {
		return heading * (math.Pi / 360)
	}
}

func (s *Supervisor) updateRef() {
	s.latitudeRef = s.Status.Latitude
	s.longitudeRef = s.Status.Longitude

	s.latitudeScale = LatitudeLen(s.Status.Latitude)
	s.longitudeScale = LongitudeLen(s.Status.Longitude)
	s.refOk = true
}

func (supervisor *Supervisor) handleSentence(sentence gps.Sentence) {
	switch sentence.(type) {
	case *gps.RMC:
		supervisor.rmcSeen.Start(time.Second)

		msg := sentence.(*gps.RMC)
		supervisor.Status.Time = msg.Time
		supervisor.Status.Latitude = msg.Latitude
		supervisor.Status.Longitude = msg.Longitude
		supervisor.Status.Speed = msg.Speed * KnotsToMs
		supervisor.Status.Track = headingToRad(msg.Track)

		if !supervisor.refOk {
			supervisor.updateRef()
		}
	case *gps.GGA:
		supervisor.ggaSeen.Start(time.Second)

		msg := sentence.(*gps.GGA)
		supervisor.Status.NumSatellites = msg.NumSatellites
	}

	log.Println("supervisor", "sentence", fmt.Sprintf("%T %v", sentence, sentence))
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

func (supervisor *Supervisor) handleInput(msg *link.Input) {
	supervisor.inputSeen.Start(time.Second)

	supervisor.Status.SteeringIn = scaleInput(msg.Channels[SteeringChannel], supervisor.inputScale)
	supervisor.Status.ThrottleIn = scaleInput(msg.Channels[ThrottleChannel], supervisor.inputScale)
	supervisor.Status.DialIn = scaleInput(msg.Channels[DialChannel], supervisor.inputScale)

	switchIn := scaleInput(msg.Channels[SwitchChannel], supervisor.inputScale)
	switch {
	case switchIn < -0.5:
		supervisor.Status.Switch = 0
	case switchIn < 0.5:
		supervisor.Status.Switch = 1
	default:
		supervisor.Status.Switch = 2
	}
}

func (supervisor *Supervisor) handleFrame(frame link.Frame) {
	switch frame.(type) {
	case *link.Input:
		supervisor.handleInput(frame.(*link.Input))
	case *link.Heartbeat:
		msg := frame.(*link.Heartbeat)
		fcpu := 8000000 + int(msg.Reference)*(1000000/64)
		supervisor.inputScale = Prescaler * 1000 * 2 / float32(fcpu)
		log.Println("supervisor", fcpu, supervisor.inputScale)
	}

	log.Println("supervisor", "frame", fmt.Sprintf("%T %v", frame, frame))
}

func (supervisor *Supervisor) step() {
	supervisor.Controller.Step(&supervisor.Status)
}

func (supervisor *Supervisor) Run() {
	tick := time.Tick(time.Millisecond * 50)

	for {
		ticked := false
		select {
		case sentence := <-supervisor.GPS.Sentences:
			supervisor.handleSentence(sentence)
		case frame := <-supervisor.Link.Frames:
			supervisor.handleFrame(frame)
		case <-tick:
			//			log.Println("tick")
			ticked = true
		}

		supervisor.Status.GpsOk = supervisor.rmcSeen.Running() && supervisor.ggaSeen.Running() && supervisor.Status.NumSatellites >= 6
		if !supervisor.ggaSeen.Running() {
			supervisor.Status.NumSatellites = -1
		}
		supervisor.Status.InputOk = supervisor.inputSeen.Running()
		if !supervisor.Status.InputOk {
			supervisor.Status.Switch = -1
		}
		supervisor.Status.Allowed = true

		if ticked {
			supervisor.step()
		}
	}
}
