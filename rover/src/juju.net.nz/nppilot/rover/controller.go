package rover

type Controller interface {
	Step(status *Status) *Demand
}
