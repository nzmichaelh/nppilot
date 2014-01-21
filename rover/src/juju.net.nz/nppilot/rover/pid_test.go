package rover

import (
	tx "juju.net.nz/testex"
	"testing"
)

const (
	dt = 0.2
)

func TestKp(t *testing.T) {
	pid := PID{Kp: 3, UMin: -100, UMax: 100}

	tx.CheckClosef(t, pid.Step(10, dt), 30)
	tx.CheckClosef(t, pid.Step(-30, dt), -90)
}

func TestKi(t *testing.T) {
	pid := PID{Ki: 4, UMin: -100, UMax: 100, TiLimit: 20}

	// Increases.
	tx.CheckClosef(t, pid.Step(10, dt), 8)
	tx.CheckClosef(t, pid.Step(10, dt), 16)
	// Stops at the TiLimit.
	tx.CheckClosef(t, pid.Step(10, dt), 20)
}

func TestULimits(t *testing.T) {
	pid := PID{Kp: 5, UMin: -50, UMax: 150}

	tx.CheckClosef(t, pid.Step(-9, dt), -45)
	tx.CheckClosef(t, pid.Step(-10, dt), -50)
	tx.CheckClosef(t, pid.Step(-11, dt), -50)

	tx.CheckClosef(t, pid.Step(29, dt), 145)
	tx.CheckClosef(t, pid.Step(30, dt), 150)
	tx.CheckClosef(t, pid.Step(31, dt), 150)
}
