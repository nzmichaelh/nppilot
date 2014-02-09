// Yet another command line parser.
package config

import (
	"bytes"
	"fmt"
	"io"
	"log"
	"os"
	"reflect"
	"strconv"
	"strings"
	"unicode"
	"unicode/utf8"
)

// Errors accumulates errors during parsing.
type Errors struct {
	errors []string
}

// Check logs an error if err is not nil.
func (e *Errors) Check(err error, format, arg1, arg2 string) {
	if err != nil {
		e.errors = append(e.errors, fmt.Sprintf(format, arg1, arg2))
	}
}

// Add adds an error to the list.
func (e *Errors) Add(format, arg string) {
	e.errors = append(e.errors, fmt.Sprintf(format, arg))
}

// Group holds a group of parameters.
type Group struct {
	Name   string
	Prefix string
	Values interface{}
}

// Config handles parsing command line arguments into structs.
//
// Groups of parameters can be stored in a struct and added as a group
// using Add().  All public, primitive fields are automatically
// exposed.  The argument names are set to the snake case version of
// the prefix and variable name.
type Config struct {
	groups []*Group
}

// Add a new group of parameters.
func (c *Config) Add(group, prefix string, values interface{}) {
	next := &Group{Name: group, Prefix: prefix, Values: values}
	c.groups = append(c.groups, next)
}

type parser func(field reflect.Value, name, value string, errors *Errors)

func parseString(field reflect.Value, name, value string, errors *Errors) {
	field.SetString(value)
}

func parseBool(field reflect.Value, name, value string, errors *Errors) {
	if value == "" {
		field.SetBool(true)
	} else {
		parsed, err := strconv.ParseBool(value)
		errors.Check(err, "Invalid boolean value %s for %s.", value, name)
		field.SetBool(parsed)
	}
}

func parseInt(field reflect.Value, name, value string, errors *Errors) {
	parsed, err := strconv.ParseInt(value, 10, 32)
	errors.Check(err, "Invalid integer value %s for %s.", value, name)
	field.SetInt(parsed)
}

func parseFloat(field reflect.Value, name, value string, errors *Errors) {
	parsed, err := strconv.ParseFloat(value, 32)
	errors.Check(err, "Invalid floating point value %s for %s.", value, name)
	field.SetFloat(parsed)
}

var parsers map[reflect.Kind]parser

func init() {
	parsers = map[reflect.Kind]parser{
		reflect.Bool: parseBool,
		reflect.Int: parseInt,
		reflect.Int32: parseInt,
		reflect.Float32: parseFloat,
		reflect.Float64: parseFloat,
		reflect.String: parseString,
	}
}

func toLower(prefix, name string) string {
	into := bytes.NewBufferString(prefix)
	chain := false

	if prefix == "" {
		chain = true
	}

	for _, ch := range name {
		if unicode.IsUpper(ch) {
			if !chain {
				into.WriteRune('_')
			}
			into.WriteRune(unicode.ToLower(ch))
			chain = true
		} else {
			into.WriteRune(ch)
			chain = false
		}
	}
	return into.String()
}

type visitor func(name string, field reflect.Value)

func visit(val interface{}, visitor visitor) {
	s := reflect.ValueOf(val).Elem()
	tp := s.Type()

	for i := 0; i < s.NumField(); i++ {
		f := s.Field(i)
		name := tp.Field(i).Name
		first, _ := utf8.DecodeRuneInString(name)
		_, hasParser := parsers[f.Kind()]

		switch {
		case !hasParser:
			break
		case !unicode.IsUpper(first):
			break
		default:
			visitor(name, f)
		}
	}
}

// Help prints human-readable help to the writer.  Use nil for
// stdout.
func (c *Config) Help(w io.Writer) {
	if w == nil {
		w = os.Stdout
	}
	for i, group := range c.groups {
		if i != 0 {
			fmt.Fprintln(w)
		}
		fmt.Fprintf(w, "%s:\n", group.Name)
		visit(group.Values, func(name string, field reflect.Value) {
			low := toLower(group.Prefix, name)
			fmt.Fprintf(w, "  --%s=%v\n", low, field.Interface())
		})
	}
}

// Print prints the parameters as a JSON dict.  Use nil for stdout.
func (c *Config) Print(w io.Writer) {
	if w == nil {
		w = os.Stdout
	}

	// Yay, another JSON encoder.
	fmt.Fprintf(w, "{\n")
	for i, group := range c.groups {
		fmt.Fprintf(w, "  %q: {\n", group.Name)
		j := 0
		visit(group.Values, func(name string, field reflect.Value) {
			if j != 0 {
				fmt.Fprintf(w, ",\n")
			}
			j += 1
			fmt.Fprintf(w, "    %q: ", name)
			switch field.Kind() {
			case reflect.String:
				fmt.Printf("%q", field.Interface())
			default:
				fmt.Printf("%v", field.Interface())
			}
		})
		if i != len(c.groups)-1 {
			fmt.Fprintf(w, "\n  },\n")
		} else {
			fmt.Fprintf(w, "\n  }\n")
		}
	}
	fmt.Fprintf(w, "}\n")
}

// Parse parses the string arguments and returns all errors seen.
func (c *Config) Parse(args []string) []string {
	errors := &Errors{}
	all := make(map[string]reflect.Value)

	for _, group := range c.groups {
		visit(group.Values, func(name string, field reflect.Value) {
			low := toLower(group.Prefix, name)

			if _, ok := all[low]; ok {
				log.Fatalf("Duplicate argument %s.", low)
			}
			all[low] = field
		})
	}

	for _, arg := range args {
		if !strings.HasPrefix(arg, "--") {
			errors.Add("Unrecognised argument %s.", arg)
			continue
		}
		name := strings.TrimPrefix(arg, "--")
		var value string
		separator := strings.Index(name, "=")

		if separator >= 0 {
			value = name[separator+1:]
			name = name[:separator]
		}

		field, ok := all[name]
		if !ok {
			errors.Add("Unrecognised argument %s.", name)
			continue
		}
		if separator < 0 && field.Kind() != reflect.Bool {
			errors.Add("%s needs an argument.", name)
		}

		parser := parsers[field.Kind()]

		if parser == nil {
			log.Fatalf("Unhandled type %s while parsing %s.", field.Kind(), name)
		}
		parser(field, name, value, errors)
	}

	return errors.errors
}

// ParseArgv parses the programs command line arguments.
func (c *Config) ParseArgv() []string {
	return c.Parse(os.Args[1:])
}

// ShowErrors prints the errors in a similar format to Write.  Use nil
// for stdout.
func (c *Config) ShowErrors(w io.Writer, errors []string) {
	if w == nil {
		w = os.Stdout
	}

	for _, error := range errors {
		fmt.Fprintf(w, "Error: %v\n", error)
	}
}
