package rover

import (
	"math"
)

const (
	m1 = 111132.92 // Latitude calculation term 1
	m2 = -559.82   // Latitude calculation term 2
	m3 = 1.175     // Latitude calculation term 3
	m4 = -0.0023   // Latitude calculation term 4
	p1 = 111412.84 // Longitude calculation term 1
	p2 = -93.5     // Longitude calculation term 2
	p3 = 0.118     // Longitude calculation term 3
)

func deg2rad(deg float64) float64 {
	return deg * math.Pi / 180
}

func LatitudeLen(deg float64) float64 {
	rad := deg2rad(deg)
	return m1 + (m2 * math.Cos(2*rad)) + (m3 * math.Cos(4*rad)) +
		(m4 * math.Cos(6*rad))
}

func LongitudeLen(deg float64) float64 {
	rad := deg2rad(deg)
	return (p1 * math.Cos(rad)) + (p2 * math.Cos(3*rad)) +
		(p3 * math.Cos(5*rad))
}
