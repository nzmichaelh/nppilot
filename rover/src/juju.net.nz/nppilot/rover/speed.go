package rover

type SpeedController struct {
	PID *PID
}

func (s *SpeedController) Step(status *Status) *Demand {
	demand := &Demand{Missing, Missing}

	// if !status.GPS.Ok || !status.Input.Ok {
	// 	s.PID.Reset()
	// 	return demand
	// }

	if status.Input.Switch <= 0 {
		s.PID.Reset()
	} else {
		var sp float32 = 0

		switch status.Input.Switch {
		case 1:
			sp = 0
		case 2:
			sp = 17.5 / 3.6
		}

		oi := s.PID.Ki
		s.PID.Ki = oi * (status.Input.Dial + 1)
		u := s.PID.Step(status.GPS.Speed, sp, Dt)
		s.PID.Ki = oi

		demand.Throttle = u
	}

	return demand
}
