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

func (pid *PID) Step(err, dt float32) float32 {
	pid.Ti = mathex.Clipf(
		pid.Ti+pid.Ki*err*dt,
		-pid.TiLimit, pid.TiLimit)
	u := pid.Kp*err+pid.Ti
	minimum := pid.Deadband * 0.1

	if u > minimum {
		u += pid.Deadband
	} else if u < -minimum {
		u -= pid.Deadband
	}

	u = mathex.Clipf(u, pid.UMin, pid.UMax)
	return u
}
