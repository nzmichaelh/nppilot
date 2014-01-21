package button

import (
	"time"
)

type Code int

const (
	None Code = iota
	Down Code = iota

	Short Code = iota
	Long  Code = iota
	Hold  Code = iota
)

const (
	holdTime = time.Second * 2
)

type Event struct {
	Code     Code
	Position int
}

type Button struct {
	Level chan int
	Event chan *Event
}

func New() *Button {
	return &Button{
		Level: make(chan int),
		Event: make(chan *Event),
	}
}

func (s *Button) Watch() {
	timer := time.NewTimer(time.Hour)
	timer.Stop()

	at := -1
	down := time.Now()

	for {
		select {
		case <-timer.C:
			s.Event <- &Event{Code: Hold, Position: at}
		case level := <-s.Level:
			switch {
			case level < 0:
				timer.Stop()
				at = level
			case level == at:
				// No change
			case at >= 0:
				elapsed := time.Since(down)

				switch {
				case elapsed < holdTime:
					s.Event <- &Event{Code: Short, Position: at}
				default:
					s.Event <- &Event{Code: Long, Position: at}
				}
				fallthrough
			default:
				s.Event <- &Event{Code: Down, Position: level}
				timer.Reset(holdTime)
				down = time.Now()
				at = level
			}
		}
	}
}
