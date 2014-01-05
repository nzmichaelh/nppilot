package rover

import (
	"log"
	"fmt"
	"time"
)

import (
	"juju.net.nz/nppilot/rover/gps"
	"juju.net.nz/nppilot/rover/link"
)

type Timeout struct {
	running bool
	end time.Time
}

func (timeout *Timeout) Start(duration time.Duration) {
	timeout.end = time.Now().Add(duration)
	timeout.running = true
}

func (timeout *Timeout) Running() bool {
	if !timeout.running {
		return false
	} else if time.Now().After(timeout.end) {
		timeout.running = false
		return false
	} else {
		return true
	}
}
	
type Status struct {
	GpsOk bool
	InputOk bool
	Allowed bool

	Switch int
}

type Supervisor struct {
	GPS *gps.Link
	Link *link.Link
	Status Status

	vtgSeen Timeout
	inputSeen Timeout
}

func (supervisor *Supervisor) handleSentence(sentence gps.Sentence) {
	switch sentence.(type) {
	case *gps.VTG:
		supervisor.vtgSeen.Start(time.Second)
	}

	log.Println("supervisor", "sentence", fmt.Sprintf("%T %v", sentence, sentence))
}

func (supervisor *Supervisor) handleInput(msg *link.Input) {
	supervisor.inputSeen.Start(time.Second)
	value := msg.Channels[4]
	switch {
	case value < -40:
		supervisor.Status.Switch = 0
	case value < 40:
		supervisor.Status.Switch = 1
	default:
		supervisor.Status.Switch = 2
	}
}

func (supervisor *Supervisor) handleFrame(frame link.Frame) {
	switch frame.(type) {
	case *link.Input:
		supervisor.handleInput(frame.(*link.Input))
	}

	log.Println("supervisor", "frame", fmt.Sprintf("%T %v", frame, frame))
}

func (supervisor *Supervisor) Run() {
	tick := time.Tick(time.Millisecond*100)

	for {
		select {
		case sentence := <- supervisor.GPS.Sentences:
			supervisor.handleSentence(sentence)
		case frame := <- supervisor.Link.Frames:
			supervisor.handleFrame(frame)
		case <- tick:
			log.Println("tick")
		}

		supervisor.Status.GpsOk = supervisor.vtgSeen.Running()
		supervisor.Status.InputOk = supervisor.inputSeen.Running()
		if !supervisor.Status.InputOk {
			supervisor.Status.Switch = -1
		}
		supervisor.Status.Allowed = true
	}
}
