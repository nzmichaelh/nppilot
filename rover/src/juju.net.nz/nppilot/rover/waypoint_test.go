package rover

import (
	tx "juju.net.nz/testex"
	_ "log"
	"math"
	"testing"
)

const (
	DegreesToM = 1.0 / 111000
)

func setup() (w *WaypointController, gps *GPS, r *Recorder, s *Status) {
	w = &WaypointController{
		Heading: PID{
			Kp: 1.0, UMax: 100, UMin: -100,
		},
		Speed: PID{
			Kp: 1.0, UMax: 100, UMin: -100,
		},
		Distance: PID{
			Kp: 1.0, UMax: 100, UMin: -100,
		},
		TargetSize: 2,
	}

	gps = &GPS{Ok: true, Latitude: 45, Longitude: 8}
	r = NewRecorder()
	s = &Status{Recorder: r, GPS: *gps}

	r.GPS(gps)
	w.GPS(gps)
	r.State = Run
	r.Mark(&r.position)
	w.Event(Run)

	return
}

func TestDistance(t *testing.T) {
	w, _, _, s := setup()
	ref := s.GPS

	// Positioned to the north by 50 m
	s.GPS.Latitude = ref.Latitude + 50*DegreesToM
	demand := w.Step(s)

	tx.CheckClosef2(t, demand.Throttle, 50, 0.3)
	tx.CheckClosef(t, demand.Steering, math.Pi)

	// To the south by 20 m
	s.GPS.Latitude = ref.Latitude - 20*DegreesToM
	demand = w.Step(s)

	tx.CheckClosef2(t, demand.Throttle, 20, 0.3)
	tx.CheckClosef(t, demand.Steering, 0)

	s.GPS.Latitude = ref.Latitude

	// To the east by 10 m
	s.GPS.Longitude = ref.Longitude + 10*DegreesToM
	demand = w.Step(s)

	tx.CheckClosef2(t, demand.Throttle, 10, 0.3)
	tx.CheckClosef(t, demand.Steering, -math.Pi/2)

	// To the west by 30 m
	s.GPS.Longitude = ref.Longitude - 30*DegreesToM
	demand = w.Step(s)

	tx.CheckClosef2(t, demand.Throttle, 30, 0.3)
	tx.CheckClosef(t, demand.Steering, math.Pi/2)
}
