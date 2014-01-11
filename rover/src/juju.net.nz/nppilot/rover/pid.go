package rover

import (
	"math"
)

type PID struct {
	Kp float64
	Ki float64
	Kd float64

	UMax    float64
	UMin    float64
	TiLimit float64

	ti float64
}

func (pid *PID) Reset() {
	pid.ti = 0
}

func (pid *PID) Step(pv, sp, dt float64) float64 {
	err := sp - pv
	pid.ti += err * dt
	pid.ti = math.Min(pid.TiLimit, math.Max(-pid.TiLimit, pid.ti))

	u := pid.Kp*err + pid.Ki*pid.ti

	switch {
	case u < pid.UMin:
		u = pid.UMin
	case u > pid.UMax:
		u = pid.UMax
	}

	return u
}
