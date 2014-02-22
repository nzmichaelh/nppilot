package link

import (
	"bytes"
	"encoding/binary"
	"io"
	"time"

	"github.com/golang/glog"
)

const (
	Mark   = '\n'
	Escape = '^'
	Xor    = 0x20
)

type Frame interface {
}

type Stats struct {
	Received uint
	Escaped  uint

	Overruns     uint
	CheckErrors  uint
	ShortFrame   uint
	Unrecognised uint
	DecodeError  uint
}

type Link struct {
	Stats Stats

	Frames chan Frame

	port io.ReadWriter
}

var now time.Time

func init() {
	now = time.Now()
}

func checksum(frame []byte) byte {
	var sum1, sum2 uint = 0x12, 0x34

	for _, ch := range frame {
		sum1 += uint(ch)
		if sum1 >= 255 {
			sum1 -= 255
		}
		sum2 += sum1
		if sum2 >= 255 {
			sum2 -= 255
		}
	}

	return byte(sum1 ^ sum2)
}

func makeMessage(code byte) interface{} {
	switch code {
	case 'h':
		return &Heartbeat{}
	case 'i':
		return &Input{}
	case 'p':
		return &Pong{}
	case 'v':
		return &Version{}
	case 's':
		return &State{}
	case 'c':
		return &Counters{}
	case 'm':
		return &IMU{}
	default:
		return nil
	}
}

func (link *Link) dispatch(frame []byte) {
	if len(frame) < 2 {
		// Need at least the code and checksum.
		link.Stats.ShortFrame += 1
	} else {
		length := len(frame) - 1
		sum := checksum(frame[:length])
		code := frame[0]
		msg := makeMessage(code)

		switch {
		case sum != frame[length]:
			// Bad checksum.
			link.Stats.CheckErrors += 1
		case msg == nil:
			link.Stats.Unrecognised += 1
		default:
			src := bytes.NewBuffer(frame[:length])
			err := binary.Read(src, binary.LittleEndian, msg)

			switch {
			case err != nil:
				link.Stats.DecodeError += 1
			case src.Len() != 0:
				link.Stats.DecodeError += 1
			default:
				link.Frames <- msg
				link.Stats.Received += 1
			}
		}
	}
}

func (link *Link) Watch() {
	got := make([]byte, 128)
	frame := &bytes.Buffer{}

	var xor byte

	for {
		n, err := link.port.Read(got)

		if n > 0 {
			for _, ch := range got[:n] {
				switch ch {
				case Mark:
					link.dispatch(frame.Bytes())
					frame.Reset()
				case Escape:
					xor = Xor
					link.Stats.Escaped += 1
				default:
					frame.WriteByte(ch ^ xor)
					xor = 0
					
					if frame.Len() > 100 {
						frame.Reset()
						link.Stats.Overruns += 1
					}
				}
			}
		}
		if n < 0 || err != nil {
			glog.Fatalf("error while reading %v, %v", n, err)
		}
	}
}

func (link *Link) Send(msg interface{}) {
	encoded := &bytes.Buffer{}
	binary.Write(encoded, binary.LittleEndian, msg)
	encoded.WriteByte(checksum(encoded.Bytes()))

	escaped := &bytes.Buffer{}
	for _, ch := range encoded.Bytes() {
		switch ch {
		case Mark, Escape:
			escaped.WriteByte(Escape)
			escaped.WriteByte(ch ^ Xor)
		default:
			escaped.WriteByte(ch)
		}
	}

	escaped.WriteByte(Mark)
	escaped.WriteTo(link.port)
}

func New(port io.ReadWriter) *Link {
	link := &Link{Frames: make(chan Frame), port: port}
	return link
}
