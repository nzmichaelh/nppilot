package textex

import (
	"math"
	"testing"
)

func CheckClose2(t *testing.T, left, right, epsilon float64) {
	err := math.Abs(left - right)

	if err > epsilon {
		t.Errorf("%v differs from %v by %v which is more than %v",
			left, right, err, epsilon)
	}
}

func CheckClose(t *testing.T, left, right float64) {
	CheckClose2(t, left, right, 0.001)
}

func CheckClosef2(t *testing.T, left, right, epsilon float32) {
	CheckClose2(t, float64(left), float64(right), float64(epsilon))
}

func CheckClosef(t *testing.T, left, right float32) {
	CheckClosef2(t, left, right, 0.001)
}
