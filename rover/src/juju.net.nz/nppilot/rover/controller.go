package rover

type Controller interface {
	Step(status *Status) *Demand
	GPS(gps *GPS)
	Event(entered State)
}
