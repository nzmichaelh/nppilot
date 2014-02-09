package config

import (
	"testing"
)

type ConfigTest struct {
	AlsoToStderr bool
	Verbosity int32
	Dir string
	Compress bool
	Threshold float32

	Baz chan int32
}

func TestHelp(t *testing.T) {
	c := &Config{}
	m := &ConfigTest{}
	c.Add("Main", "main", m)
	a := &ConfigTest{}
	c.Add("Another", "a", a)
	c.Help(nil)
}

func TestParse(t *testing.T) {
	c := &Config{}
	m := &ConfigTest{}
	c.Add("Log", "log", m)

	errs := c.Parse([]string{"--log_also_to_stderr=true"})
	if len(errs) != 0 { t.Error(errs) }
	if m.AlsoToStderr != true { t.Error() }

	// Booleans may also be naked.
	errs = c.Parse([]string{"--log_compress"})
	if len(errs) != 0 { t.Error(errs) }
	if m.Compress != true { t.Error() }

	errs = c.Parse([]string{"--log_verbosity=17"})
	if len(errs) != 0 { t.Error(errs) }
	if m.Verbosity != 17 { t.Error() }

	errs = c.Parse([]string{"--log_threshold=0.5"})
	if len(errs) != 0 { t.Error(errs) }
	if m.Threshold != 0.5 { t.Error() }

	errs = c.Parse([]string{"--log_dir=/foo/bar"})
	if len(errs) != 0 { t.Error(errs) }
	if m.Dir != "/foo/bar" { t.Error() }
}
