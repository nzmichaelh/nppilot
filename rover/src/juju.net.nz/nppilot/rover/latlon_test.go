package rover

import (
	tx "juju.net.nz/testex"
	"testing"
)

func TestLatitude(t *testing.T) {
	tx.CheckClose2(t, LatitudeLen(0), 110574, 1)
	tx.CheckClose2(t, LatitudeLen(15), 110649, 1)
	tx.CheckClose2(t, LatitudeLen(30), 110852, 1)
	tx.CheckClose2(t, LatitudeLen(45), 111132, 1)
	tx.CheckClose2(t, LatitudeLen(60), 111412, 1)
	tx.CheckClose2(t, LatitudeLen(75), 111618, 1)
	tx.CheckClose2(t, LatitudeLen(90), 111694, 1)
}

func TestLongitude(t *testing.T) {
	tx.CheckClose2(t, LongitudeLen(0), 111320, 1)
	tx.CheckClose2(t, LongitudeLen(15), 107550, 1)
	tx.CheckClose2(t, LongitudeLen(30), 96486, 1)
	tx.CheckClose2(t, LongitudeLen(45), 78847, 1)
	tx.CheckClose2(t, LongitudeLen(60), 55800, 1)
	tx.CheckClose2(t, LongitudeLen(75), 28902, 1)
	tx.CheckClose2(t, LongitudeLen(90), 0.000, 1)
}
