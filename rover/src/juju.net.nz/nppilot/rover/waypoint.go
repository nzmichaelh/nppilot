package rover

import (
)

type WaypointController struct {
	Heading *PID
	Speed   *PID
	Distance *PID
	TargetSize float32

	position Point
	waypoint int
	scale    Point
}

type WaypointState struct {
	Waypoint int
	Distance float32
	Heading float32
	Speed float32
}

func (w *WaypointController) Step(status *Status) *Demand {
	demand := &Demand{Missing, Missing}
	points := status.Recorder.Points

	if status.Recorder.State != Run {
		return demand
	}
	if w.waypoint < 0 || w.waypoint >= len(points) {
		w.waypoint = -1
		return demand
	}

	position := &Point{X: status.GPS.Latitude, Y: status.GPS.Longitude}
	target := points[w.waypoint]
	diffll := target.Sub(position)
	diff := diffll.Mul(&w.scale)

	distance := float32(diff.Mag())
	heading := float32(diff.Angle())
	speed := w.Distance.Step(distance, Dt)
	demand.Throttle = w.Speed.Step(speed - status.GPS.Speed, Dt)
	demand.Steering = w.Heading.Step(
		HeadingErr(heading, status.GPS.Track), Dt)

	Info("waypoint", &WaypointState{Waypoint: w.waypoint, Distance: distance, Heading: heading, Speed: speed})

	if distance < w.TargetSize {
		w.waypoint++
	}

	return demand
}

func (w *WaypointController) GPS(status *Status) {
	w.position.X = status.GPS.Latitude
	w.position.Y = status.GPS.Longitude
}

func (w *WaypointController) Event(entered State) {
	switch entered {
	case Run:
		w.scale.X = LatitudeLen(w.position.X)
		w.scale.Y = LongitudeLen(w.position.Y)
		if w.waypoint < 0 {
			w.waypoint = 0
		}
	}
}
