package rover

import (
	tx "juju.net.nz/testex"
	"testing"
)

type point struct {
	Latitude  float64
	Longitude float64
	Eastings  float64
	Northings float64
}

func TestFromLatLon(t *testing.T) {
	known_values := [...]point{
		// Aachen, Germany
		point{50.77535, 6.08389, 294409, 5628898},
		// New York, USA
		point{40.71435, -74.00597, 583960, 4507523},
		// Wellington, New Zealand
		point{-41.28646, 174.77624, 313784, 5427057},
		// Capetown, South Africa
		point{-33.92487, 18.42406, 261878, 6243186},
		// Mendoza, Argentina
		point{-32.89018, -68.84405, 514586, 6360877},
		// Fairbanks, Alaska, USA
		point{64.83778, -147.71639, 466013, 7190568},
		// Ben Nevis, Scotland, UK
		point{56.79680, -5.00601, 377486, 6296562},
	}

	for _, point := range known_values {
		eastings, northings := FromLatLon(point.Latitude, point.Longitude)
		tx.CheckClose2(t, point.Eastings, eastings, 1)
		tx.CheckClose2(t, point.Northings, northings, 1)
	}
}
