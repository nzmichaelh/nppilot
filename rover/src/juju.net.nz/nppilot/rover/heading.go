package rover

import (
	"math"
)

type HeadingController struct {
	PID *PID

	sp float32
}

type HeadingState struct {
	Switch int
	Kp float32
	Ki float32
	SP float32
	PV float32
	Err float32
	U float32
}

func HeadingErr(sp, pv float32) float32 {
	err := sp - pv
	if err > math.Pi {
		err -= 2*math.Pi
	} else if err < -math.Pi {
		err += 2*math.Pi
	}

	return err
}

func (c *HeadingController) Step(status *Status) *Demand {
	demand := &Demand{Missing, Missing}

	// if !status.GPS.Ok || !status.Input.Ok {
	// 	s.PID.Reset()
	// 	return demand
	// }
	pv := status.GPS.Track
	sp := c.sp

	switch {
	case status.Input.Steering < -0.5:
		sp -= math.Pi/2
	case status.Input.Steering < -0.25:
		sp -= math.Pi/4
	case status.Input.Steering > 0.5:
		sp += math.Pi/2
	case status.Input.Steering > 0.25:
		sp += math.Pi/4
	}
	if sp > math.Pi {
		sp -= math.Pi*2
	} else if sp < -math.Pi {
		sp += math.Pi*2
	}

	err := HeadingErr(sp, pv)
	op := c.PID.Kp
	c.PID.Kp = status.Input.Dial * op
	u := c.PID.Step(err, Dt)

	Info("heading", &HeadingState{Switch: status.Input.Switch, Kp: c.PID.Kp, Ki: c.PID.Ki, SP: sp, PV: pv, Err: err, U:u})
	c.PID.Kp = op

	switch status.Input.Switch {
	case 2:
		demand.Steering = u
	default:
		c.sp = pv
		c.PID.Reset()
	}

	return demand
}

func (w *HeadingController) GPS(gps *GPS) {
}

func (w *HeadingController) Event(entered State) {
}
