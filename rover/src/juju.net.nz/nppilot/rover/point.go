package rover

import (
	"math"
)

type Point struct {
	X float64
	Y float64
}

func (p *Point) Mul(other *Point) *Point {
	return &Point{
		X: p.X*other.X,
		Y: p.Y*other.Y,
	}
}

func (p *Point) Sub(other *Point) *Point {
	return &Point{
		X: p.X-other.X,
		Y: p.Y-other.Y,
	}
}

func (p *Point) Mag() float64 {
	return math.Sqrt(p.X*p.X + p.Y*p.Y)
}

func (p *Point) Angle() float64 {
	return math.Atan2(p.Y, p.X)
}
