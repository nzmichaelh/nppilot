package rover

import (
	"container/list"
	"github.com/golang/glog"
	"juju.net.nz/nppilot/rover/button"
)

type State int

const (
	Drive  = iota
	Record = iota
	Run    = iota
	Stop   = iota
)

type Point struct {
	Latitude  float64
	Longitude float64
}

type Recorder struct {
	state  State
	points list.List

	position Point
	target   Point
}

func (r *Recorder) Switch(event *button.Event) {
	glog.V(2).Infof("recorder: switch: %+v in %v\n", event, r.state)

	switch {
	case r.state == Drive && false:
		return
	case event.Code == button.Hold && event.Position == 0:
		r.points.Init()
		r.state = Record
		glog.V(1).Infof("recorder: reset\n")
	case event.Code == button.Short && event.Position == 0 && r.state == Record:
		dup := r.position
		r.points.PushBack(dup)
		glog.V(1).Infof("recorder: mark: %+v\n", dup)
	case event.Code == button.Down && event.Position == 2 && r.state == Record:
		r.state = Run
		glog.V(1).Infof("recorder: run\n")
	case event.Code == button.Down && event.Position == 1 && r.state == Run:
		r.state = Record
		glog.V(1).Infof("recorder: pause\n")
	}
}

func (r *Recorder) GPS(gps *GPS) {
	r.position.Latitude = gps.Latitude
	r.position.Longitude = gps.Longitude

	switch {
	case gps.Ok && r.state == Drive:
		r.points.Init()
		r.state = Record
	case !gps.Ok:
		r.state = Drive
	}
}
