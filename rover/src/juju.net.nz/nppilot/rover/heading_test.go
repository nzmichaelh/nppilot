package rover

import (
	tx "juju.net.nz/testex"
	"testing"
)

func TestHeading(t *testing.T) {
	c := &HeadingController{
		PID: &PID{Kp: 1, UMax: 10, UMin: -10}}

	s := &Status{}
	s.Input.Switch = 1
	s.GPS.Track = 0

	// Save the heading as the set point.
	c.Step(s)
	s.Input.Switch = 2
	// On track.
	s.GPS.Track = 0
	tx.CheckClosef(t, c.Step(s).Steering, 0)
	// Actual heading is to the west.  Expect turn to the left.
	s.GPS.Track = 1
	tx.CheckClosef(t, c.Step(s).Steering, -1)
	// Actual heading is to the east.  Expect turn to the right.
	s.GPS.Track = -1
	tx.CheckClosef(t, c.Step(s).Steering, 1)
}

func TestWrap(t *testing.T) {
	c := &HeadingController{
		PID: &PID{Kp: 1, UMax: 10, UMin: -10}}

	s := &Status{}
	s.Input.Switch = 1
	s.GPS.Track = 2

	// Save the heading as the set point.
	c.Step(s)
	s.Input.Switch = 2

	s.GPS.Track = 1
	tx.CheckClosef(t, c.Step(s).Steering, 1)
	s.GPS.Track = -1
	tx.CheckClosef(t, c.Step(s).Steering, 3)
	// Still quicker to turn left.
	s.GPS.Track = -1.14
	tx.CheckClosef(t, c.Step(s).Steering, 3.14)

	s.GPS.Track = 3
	tx.CheckClosef(t, c.Step(s).Steering, -1)
	// Wrap around.  Pi-2 to the wrap point, then Pi-3 to -3.
	s.GPS.Track = -3
	tx.CheckClosef(t, c.Step(s).Steering, -(1.1415+0.1415))
	// Almost on the bounary.
	s.GPS.Track = -1.2
	tx.CheckClosef(t, c.Step(s).Steering, -3.083)
}
