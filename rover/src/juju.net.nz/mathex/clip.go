package mathex

// Clip a value between limits.
func Clipf(v, min, max float32) float32 {
	switch {
	case v < min:
		return min
	case v > max:
		return max
	default:
		return v
	}
}
