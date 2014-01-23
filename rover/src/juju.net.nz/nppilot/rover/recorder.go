package rover

import (
	"github.com/golang/glog"
	"juju.net.nz/nppilot/rover/button"
)

type State int

const (
	None = iota
	Drive  = iota
	Record = iota
	Run    = iota
	Stop   = iota
)

type Recorder struct {
	State  State
	Points []Point

	position Point
}

func NewRecorder() *Recorder {
	return &Recorder{
		State: Drive,
		Points: make([]Point, 0),
	}
}

func (r *Recorder) Mark(position *Point) {
	r.Points = append(r.Points, *position)
}

func (r *Recorder) Clear() {
	r.Points = make([]Point, 0)
}

func (r *Recorder) Switch(event *button.Event) State {
	start := r.State

	glog.V(2).Infof("recorder: switch: %+v in %v\n", event, r.State)

	switch {
	case r.State == Drive && false:
		// Keep driving.
	case event.Code == button.Hold && event.Position == 0:
		r.Clear()
		r.State = Record
		glog.V(1).Infof("recorder: reset\n")
	case event.Code == button.Short && event.Position == 0 && r.State == Record:
		r.Mark(&r.position)
		glog.V(1).Infof("recorder: mark: %+v\n", r.position)
	case event.Code == button.Down && event.Position == 2 && r.State == Record:
		r.State = Run
		glog.V(1).Infof("recorder: run\n")
	case event.Code == button.Down && event.Position == 1 && r.State == Run:
		r.State = Record
		glog.V(1).Infof("recorder: pause\n")
	}

	if start != r.State {
		return r.State
	} else {
		return None
	}
}

func (r *Recorder) GPS(gps *GPS) {
	r.position.X = gps.Latitude
	r.position.Y = gps.Longitude

	switch {
	case gps.Ok && (r.State == None || r.State == Drive):
		r.Clear()
		r.State = Record
	case !gps.Ok:
		r.State = Drive
	}
}
