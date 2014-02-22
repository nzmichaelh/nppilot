package rover

import (
	"juju.net.nz/mathex"
)

type Jogger struct {
	value *float32
	original float32
}

func NewJogger(value *float32, dial, offset, scale float32) *Jogger {
	j := &Jogger{value: value, original: *value}

	dial = mathex.Clipf(dial, -1, 1)

	v := *value + *value * (dial + offset) * scale
	if v < 0 {
		v = 0
	}
	*value = v
	return j
}

func (j *Jogger) Restore() {
	*j.value = j.original
}
