package rover

import (
	"time"
	"github.com/golang/glog"
)

const (
	limit = time.Second
)

type Watchdog struct {
	Name string

	last time.Time
	seen bool
}

type event struct {
	Name string
	Ok bool
	Elapsed int
}

func (w *Watchdog) mark() bool {
	since := time.Since(w.last)
	ok := since <= limit

	event := &event{
		Name: w.Name,
		Ok: ok,
		Elapsed: int(since.Seconds()),
	}
	Info("watchdog", event)

	return ok
}

func (w *Watchdog) Feed() {
	w.last = time.Now()

	if !w.seen {
		w.seen = true
		w.mark()
	}
}

func (w *Watchdog) Check() {
	if !w.mark() && w.seen {
		w.seen = false
		glog.V(1).Infof("watchdog: %v lost", w.Name)
	}
}
