package rover

import (
	"github.com/golang/glog"
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

	glog.V(2).Infof("waypoint: n:%v distance:%v heading:%v speed:%v\n", w.waypoint, distance, heading, speed)

	if distance < w.TargetSize {
		w.waypoint++
	}

	return demand
}

func (w *WaypointController) GPS(gps *GPS) {
	w.position.X = gps.Latitude
	w.position.Y = gps.Longitude
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
