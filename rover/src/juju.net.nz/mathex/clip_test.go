package mathex

import (
	tx "juju.net.nz/testex"
	"testing"
)

func TestClipf(t *testing.T) {
	tx.CheckClosef(t, Clipf(0.7, 0.5, 1.0), 0.7)
	tx.CheckClosef(t, Clipf(1.1, 0.5, 1.0), 1.0)
	tx.CheckClosef(t, Clipf(0.3, 0.5, 1.0), 0.5)
}
