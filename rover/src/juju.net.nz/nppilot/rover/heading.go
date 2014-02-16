package rover

import (
	"math"
)

const (
	GpsDt = 0.2
	Engaged = 2
)

type HeadingController struct {
	PID *PID

	u float32
	sp float32
}

type HeadingState struct {
	Switch int
	Kp float32
	Ki float32
	SP float32
	PV float32
	Err float32
	Ti float32
	Td float32
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

	if status.Input.Switch == Engaged {
		demand.Steering = c.u
	}

	return demand
}

func (c *HeadingController) GPS(status *Status) {
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
	op := c.PID.Kd
	c.PID.Kd = (status.Input.Dial+1)*5 * op
	u := c.PID.Step(err, GpsDt)

	Info("heading", &HeadingState{Switch: status.Input.Switch, Kp: c.PID.Kp, Ki: c.PID.Ki, SP: sp, PV: pv, Err: err, Ti: c.PID.Ti, Td: c.PID.Td, U:u})
	c.PID.Kd = op

	c.u = u

	if status.Input.Switch != Engaged {
		c.sp = pv
		c.PID.Reset()
	}
}

func (c *HeadingController) Event(entered State) {
}
