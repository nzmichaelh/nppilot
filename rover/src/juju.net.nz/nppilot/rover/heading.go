package rover

import (
	"math"
)

type HeadingController struct {
	PID *PID

	sp float32
}

func (c *HeadingController) Step(status *Status) *Demand {
	demand := &Demand{Missing, Missing}

	// if !status.GPS.Ok || !status.Input.Ok {
	// 	s.PID.Reset()
	// 	return demand
	// }
	pv := status.Input.Dial*3

	switch status.Input.Switch {
	case 1:
		c.sp = pv
	case 2:
		err := c.sp - pv
		if err > math.Pi {
			err -= 2*math.Pi
		} else if err < -math.Pi {
			err += 2*math.Pi
		}
//		oi := c.PID.Ki
//		c.PID.Ki = oi * (status.Input.Dial + 1)
		u := c.PID.Step(err, Dt)
//		c.PID.Ki = oi

		demand.Steering = u
	default:
		c.PID.Reset()
	}

	return demand
}
