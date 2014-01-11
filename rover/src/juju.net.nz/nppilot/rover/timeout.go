package rover

import "time"

type Timeout struct {
	running bool
	end     time.Time
}

func (timeout *Timeout) Start(duration time.Duration) {
	timeout.end = time.Now().Add(duration)
	timeout.running = true
}

func (timeout *Timeout) Stop() {
	timeout.running = false
}

func (timeout *Timeout) Running() bool {
	if !timeout.running {
		return false
	} else if time.Now().After(timeout.end) {
		timeout.running = false
		return false
	} else {
		return true
	}
}
