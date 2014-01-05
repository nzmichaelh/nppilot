package gps

import (
	"bytes"
	"errors"
	"io"
	"log"
	"regexp"
	"strconv"
	"strings"
	"fmt"
)

var matcher *regexp.Regexp

func init() {
	matcher = regexp.MustCompile(`\$([A-Z]+,.+)\*(\w\w)$`)
}

type Message interface {
}

type GGA struct {
	Time         float64
	Latitude     float64
	Longitude    float64
	Quality      int
	NumSatelites int
	HDOP         float64
	Altitude     float64
}

type VTG struct {
	TrueCourse float64
	// MagneticCourse float64
	Speed      float64
	SpeedInKmh float64
	Mode       string
}

type RMC struct {
	Time      float64
	Status    string
	Latitude  float64
	Longitude float64
	Speed     float64
	Track     float64
	Date      string
	Mode      string
}

type Link struct {
	Sentences        uint
	UnrecognisedType uint
	ChecksumError    uint
	ParseError       uint
	EmptyFields      uint
	DecodeError      uint
}

type Parser struct {
	fields []string
	EmptyField bool
	Err    error
}

func (parser *Parser) setError(text string) {
	if parser.Err == nil {
		parser.Err = errors.New(text)
	}
}

func (parser *Parser) updateError(err error) {
	if err != nil {
		parser.Err = err
	}
}

func (parser *Parser) get(idx int, fallback string) string {
	if idx >= len(parser.fields) {
		parser.setError("Field out of range.")
		return fallback
	} else {
		value := parser.fields[idx]
		if value == "" {
			parser.EmptyField = true
			return fallback
		} else {
			return value
		}
	}

}

func (parser *Parser) parseInt(value string) int {
	f, err := strconv.ParseInt(value, 0, 0)
	parser.updateError(err)
	return int(f)
}

func (parser *Parser) parseFloat(value string) float64 {
	f, err := strconv.ParseFloat(value, 64)
	parser.updateError(err)
	return f
}

func (parser *Parser) GetString(idx int) string {
	return parser.get(idx, "")
}

func (parser *Parser) GetInt(idx int) int {
	return parser.parseInt(parser.get(idx, "0"))
}

func (parser *Parser) GetFloat(idx int) float64 {
	value := parser.get(idx, "0")
	f, err := strconv.ParseFloat(value, 64)
	parser.updateError(err)
	return f
}

func (parser *Parser) GetTime(idx int) float64 {
	value := parser.parseFloat(parser.GetString(idx))
	hhmm := int(value / 100)
	hours, minutes := hhmm/100, hhmm%100
	seconds := value - float64(hhmm*100)

	return float64(hours*3600+minutes*60) + seconds
}

func (parser *Parser) GetLatLong(idx int) float64 {
	value := parser.GetFloat(idx)
	dd := int(value / 100)
	mm := value - float64(dd*100)
	decimal := float64(dd) + mm/60

	direction := parser.get(idx+1, "W")

	if direction == "W" || direction == "S" {
		decimal = -decimal
	}

	return decimal
}

func checksum(frame string) uint8 {
	var sum uint8
	for _, ch := range frame {
		sum ^= uint8(ch)
	}
	return sum
}

func (link *Link) decode(frame string) (msg Message, err error, emptyField bool) {
	parser := &Parser{strings.Split(frame, ","), false, nil}
	log.Println(parser)

	switch parser.GetString(0) {
	case "GPGGA":
		msg = &GGA{
			Time:         parser.GetTime(1),
			Latitude:     parser.GetLatLong(2),
			Longitude:    parser.GetLatLong(4),
			Quality:      parser.GetInt(6),
			NumSatelites: parser.GetInt(7),
			HDOP:         parser.GetFloat(8),
			Altitude:     parser.GetFloat(9),
		}
	case "GPVTG":
		msg = &VTG{
			TrueCourse: parser.GetFloat(1),
			Speed:      parser.GetFloat(5),
			SpeedInKmh: parser.GetFloat(7),
			Mode:       parser.GetString(9),
		}
	case "GPRMC":
		msg = &RMC{
			Time:      parser.GetTime(1),
			Status:    parser.GetString(2),
			Latitude:  parser.GetLatLong(3),
			Longitude: parser.GetLatLong(5),
			Speed:     parser.GetFloat(7),
			Track:     parser.GetFloat(8),
			Date:      parser.GetString(9),
			Mode:      parser.GetString(12),
		}
	}

	return msg, parser.Err, parser.EmptyField
}

func (link *Link) dispatch(frame *bytes.Buffer) {
	if frame.Len() == 0 {
		return
	}

	parts := matcher.FindStringSubmatch(frame.String())

	if len(parts) == 0 {
		link.ParseError += 1
		return
	}

	link.Sentences += 1
	sum := checksum(parts[1])
	got, err := strconv.ParseUint(parts[2], 16, 8)

	if err != nil || uint8(got) != sum {
		link.ChecksumError += 1
		return
	}

	msg, err, emptyField := link.decode(parts[1])

	if err != nil {
		link.DecodeError += 1
	} else if emptyField {
		link.EmptyFields += 1
	} else if msg != nil {
		fmt.Printf("%T %v\n", msg, msg)
	}
}

func (link *Link) Read(port io.Reader) {
	got := make([]byte, 128)
	var frame bytes.Buffer

	for {
		n, err := port.Read(got)

		if n > 0 {
			for _, ch := range got[:n] {
				switch ch {
				case '$':
					frame.Reset()
					frame.WriteByte(ch)
				case '\r':

				case '\n':
					link.dispatch(&frame)
					frame.Reset()
				default:
					frame.WriteByte(ch)
				}
			}
		}
		if n == 0 || err != nil {
			break
		}
	}
}

func (link *Link) Watch(port io.Reader) {
	link.Read(port)
}

