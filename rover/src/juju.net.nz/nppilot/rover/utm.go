// Convert latitude and longitude to UTM-WGS84 eastings and northings.
//
// A fairly mechanical translation of utm.py by Tobias Bieniek.

package rover

import (
	"math"
)

const (
	K0 = 0.9996

	E    = 0.00669438
	E2   = E * E
	E3   = E2 * E
	E_P2 = E / (1.0 - E)

	// SQRT_E = math.Sqrt(1 - E)
	SQRT_E = 0.9966471893303066
	_E     = (1 - SQRT_E) / (1 + SQRT_E)
	_E3    = _E * _E * _E
	_E4    = _E3 * _E

	M1 = (1 - E/4 - 3*E2/64 - 5*E3/256)
	M2 = (3*E/8 + 3*E2/32 + 45*E3/1024)
	M3 = (15*E2/256 + 45*E3/1024)
	M4 = (35 * E3 / 3072)

	P2 = (3*_E/2 - 27*_E3/32)
	P3 = (21*_E3/16 - 55*_E4/32)
	P4 = (151 * _E3 / 96)

	R = 6378137
)

func radians(degrees float64) float64 {
	return degrees * math.Pi / 180
}

func latLonToZoneNumber(latitude, longitude float64) int {
	if latitude >= 56 && latitude <= 64 && longitude >= 3 && longitude <= 12 {
		return 32
	}

	if latitude >= 72 && latitude <= 84 && longitude >= 0 {
		switch {
		case longitude <= 9:
			return 31
		case longitude <= 21:
			return 33
		case longitude <= 33:
			return 35
		case longitude <= 42:
			return 37
		}
	}

	return int((longitude+180)/6) + 1
}

func zoneNumberToCentralLongitude(zone_number int) int {
	return (zone_number-1)*6 - 180 + 3
}

// Convert a latitude, longitude into UTM northings and eastings.
func FromLatLon(latitude, longitude float64) (easting, northing float64) {
	lat_rad := radians(latitude)
	lat_sin := math.Sin(lat_rad)
	lat_cos := math.Cos(lat_rad)

	lat_tan := lat_sin / lat_cos
	lat_tan2 := lat_tan * lat_tan
	lat_tan4 := lat_tan2 * lat_tan2

	lon_rad := radians(longitude)

	zone_number := latLonToZoneNumber(latitude, longitude)
	central_lon := zoneNumberToCentralLongitude(zone_number)
	central_lon_rad := radians(float64(central_lon))

	n := R / math.Sqrt(1-E*lat_sin*lat_sin)
	c := E_P2 * lat_cos * lat_cos

	a := lat_cos * (lon_rad - central_lon_rad)
	a2 := a * a
	a3 := a2 * a
	a4 := a3 * a
	a5 := a4 * a
	a6 := a5 * a

	m := R * (M1*lat_rad -
		M2*math.Sin(2*lat_rad) +
		M3*math.Sin(4*lat_rad) -
		M4*math.Sin(6*lat_rad))

	easting = K0*n*(a+
		a3/6*(1-lat_tan2+c)+
		a5/120*(5-18*lat_tan2+lat_tan4+72*c-58*E_P2)) + 500000

	northing = K0 * (m + n*lat_tan*(a2/2+
		a4/24*(5-lat_tan2+9*c+4*c*c)+
		a6/720*(61-58*lat_tan2+lat_tan4+600*c-330*E_P2)))

	if latitude < 0 {
		northing += 10000000
	}

	return
}
