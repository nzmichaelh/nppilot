package rover

import (
	"juju.net.nz/mathex"
)

type PID struct {
	Kp float32
	Ki float32
	Kd float32

	Deadband float32

	UMax    float32
	UMin    float32
	TiLimit float32

	Ti float32
}

func (pid *PID) Reset() {
	pid.Ti = 0
}

func (pid *PID) Step(pv, sp, dt float32) float32 {
	err := sp - pv

	pid.Ti = mathex.Clipf(
		pid.Ti + pid.Ki * err * dt,
		-pid.TiLimit, pid.TiLimit)
	u := mathex.Clipf(
		pid.Kp*err + pid.Ti,
		pid.UMin, pid.UMax)

	switch {
	case pid.Deadband == 0:
		break
	case u > pid.Deadband * 0.1:
		u += pid.Deadband
	}

	return u
}
