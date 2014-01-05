package link

import (
        "log"
	"io"
	"encoding/binary"
	"bytes"
	"time"
	"fmt"
)

const (
	Mark = '\n'
	Escape = '^'
	Xor = 0x20
)

type Link struct {
	Received uint
	Escaped uint

	Overruns uint
	CheckErrors uint
	ShortFrame uint
	Unrecognised uint
	DecodeError uint
}

type Heartbeat struct {
	Code uint8
	Version uint8
	DeviceId uint8
	Ticks uint8
}

type Input struct {
	Code uint8
	Channels [6]int8
}

type Pong struct {
	Code uint8
}

type Request struct {
	Code uint8
	Requested uint8
}

type Version struct {
	Code uint8
	Version [18]byte
}

type State struct {
	Code uint8
	Flags uint8	
}

type Demand struct {
	Code uint8
	Flags uint8
	Channels [6]int8
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

func make_message(code byte) interface{} {
	switch code {
	case 'h':
		return &Heartbeat{}
	case 'i':
		return &Input{}
	case 'P':
		return &Pong{}
	case 'v':
		return &Version{}
	case 's':
		return &State{}
	default:
		return nil
	}
}

func (link *Link) dispatch(frame []byte) {
	if len(frame) < 2 {
		// Need at least the code and checksum.
		link.ShortFrame += 1
		log.Println("err: shortframe")
	} else {
//		log.Println("frame:", frame)

		length := len(frame) - 1
		sum := checksum(frame[:length])
		code := frame[0]
		msg := make_message(code)

		switch {
		case sum != frame[length]:
			// Bad checksum.
			link.CheckErrors += 1
			log.Println("err: checksum", sum, frame[length])
		case msg == nil:
			log.Println("err: unrecognised", code)
			link.Unrecognised += 1
		default:
			src := bytes.NewBuffer(frame[:length])
			err := binary.Read(src, binary.LittleEndian, msg)

			switch {
			case err != nil:
				link.DecodeError += 1
				log.Println("err: decode error", err, binary.Size(msg), length)
			case src.Len() != 0:
				link.DecodeError += 1
				log.Println("err: unused input while decoding.")
			default:
				log.Println(fmt.Sprintf("%.3f", time.Now().Sub(now).Seconds()), "ok", string(code), msg)
				link.Received += 1
			}
		}
	}
}

func (link *Link) Watch(port io.ReadWriter) {
	got := make([]byte, 1)
        frame := make([]byte, 0)
	rx := make([]byte, 0)

        var xor byte

	for {
	        n, err := port.Read(got)
        	if err != nil {
                   log.Fatal(err)
		}

                for _, ch := range got[:n] {
			rx = append(rx, ch)
			switch ch {
			case Mark:
//				log.Println("rx", len(rx), rx)
				link.dispatch(frame)
				frame = make([]byte, 0)
				rx = make([]byte, 0)
			case Escape:
				xor = Xor
				link.Escaped += 1
			default:
				frame = append(frame, ch ^ xor)
				xor = 0

				if len(frame) > 100 {
					link.Overruns += 1
					log.Println("err: overrun")
					frame = make([]byte, 0)
				}
                        }
                }
	}
}

func (link *Link) Send(port io.ReadWriteCloser, msg interface{}) {
	encoded := new(bytes.Buffer)
	binary.Write(encoded, binary.LittleEndian, msg)
	encoded.WriteByte(checksum(encoded.Bytes()))

	log.Println("send", encoded.Bytes())

	escaped := make([]byte, 0)

	for _, ch := range encoded.Bytes() {
		switch ch {
		case Mark, Escape:
			escaped = append(escaped, Escape)
			escaped = append(escaped, ch ^ Xor)
		default:
			escaped = append(escaped, ch)
		}
	}

	escaped = append(escaped, Mark)
	port.Write(escaped)
}
