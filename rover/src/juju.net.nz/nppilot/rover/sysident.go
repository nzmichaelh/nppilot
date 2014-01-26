package rover

type SysIdentController struct {
	PID *PID
}

func (s *SysIdentController) Step(status *Status) *Demand {
	demand := &Demand{Missing, Missing}

	if status.Input.Switch <= 0 {
	} else {
		var u float32 = 0

		switch status.Input.Switch {
		case 1:
			u = 0
		case 2:
			u = (status.Input.Dial + 1) / 2
		}

		demand.Throttle = u
	}

	return demand
}

func (w *SysIdentController) GPS(gps *GPS) {
}

func (w *SysIdentController) Event(entered State) {
}
