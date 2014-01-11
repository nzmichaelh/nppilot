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

	tx.CheckClose(t, pid.Step(10, 20, dt), 30)
	tx.CheckClose(t, pid.Step(20, -10, dt), -90)
}

func TestKi(t *testing.T) {
	pid := PID{Ki: 4, UMin: -100, UMax: 100, TiLimit: 5}

	// Increases.
	tx.CheckClose(t, pid.Step(0, 10, dt), 8)
	tx.CheckClose(t, pid.Step(0, 10, dt), 16)
	// Stops at the TiLimit.
	tx.CheckClose(t, pid.Step(0, 10, dt), 20)
}

func TestULimits(t *testing.T) {
	pid := PID{Kp: 5, UMin: -50, UMax: 150}

	tx.CheckClose(t, pid.Step(0, -9, dt), -45)
	tx.CheckClose(t, pid.Step(0, -10, dt), -50)
	tx.CheckClose(t, pid.Step(0, -11, dt), -50)

	tx.CheckClose(t, pid.Step(0, 29, dt), 145)
	tx.CheckClose(t, pid.Step(0, 30, dt), 150)
	tx.CheckClose(t, pid.Step(0, 31, dt), 150)
}
