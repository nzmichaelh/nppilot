package rover

type Controller interface {
	Step(status *Status) *Demand
	GPS(status *Status)
	Event(entered State)
}
