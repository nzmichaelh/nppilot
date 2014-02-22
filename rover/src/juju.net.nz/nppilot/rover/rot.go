package rover

type ROTController struct {
	PID *PID
}

type ROTState struct {
	Kp float32
	Ki float32
	SP float32
	PV float32
	Ti float32
	Td float32
	U float32
}

func (s *ROTController) Step(status *Status) *Demand {
	demand := &Demand{Missing, Missing}

	if status.Input.Switch <= 0 {
		s.PID.Reset()
	} else {
		j := NewJogger(&s.PID.Ki, status.Input.Dial, 0, 3)
		defer j.Restore()

		sp := float32(int(status.Input.Steering*3)) * 1.0
		u := s.PID.Step(sp - status.IMU.RateOfTurn, Dt)
		demand.Steering = u

		Info("rot", &ROTState{
			Kp: s.PID.Kp,
			Ki: s.PID.Ki,
			SP: sp,
			PV: status.IMU.RateOfTurn,
			Ti: s.PID.Ti,
			Td: s.PID.Td,
			U: u,
		})
	}

	return demand
}

func (w *ROTController) GPS(status *Status) {
}

func (w *ROTController) Event(entered State) {
}
