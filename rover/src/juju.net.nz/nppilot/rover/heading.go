package rover

import (
	"math"
	"github.com/golang/glog"
)

type HeadingController struct {
	PID *PID

	sp float32
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

	switch {
	case status.Input.Steering < -0.5:
		pv -= math.Pi/2
	case status.Input.Steering < -0.25:
		pv -= math.Pi/4
	case status.Input.Steering > 0.5:
		pv += math.Pi/2
	case status.Input.Steering > 0.25:
		pv += math.Pi/4
	}
	if pv > math.Pi {
		pv -= math.Pi*2
	} else if pv < -math.Pi {
		pv += math.Pi*2
	}

	switch status.Input.Switch {
	case 1:
		c.sp = pv
	case 2:
		err := HeadingErr(c.sp, pv)
		oi := c.PID.Ki
		c.PID.Ki = oi * (status.Input.Dial + 1)
		u := c.PID.Step(err, Dt)
		c.PID.Ki = oi

		glog.V(2).Infof("heading: sp %v pv %v err %v u %v\n", c.sp, pv, err, u)
		demand.Steering = u
	default:
		c.PID.Reset()
	}

	return demand
}

func (w *HeadingController) GPS(gps *GPS) {
}

func (w *HeadingController) Event(entered State) {
}
