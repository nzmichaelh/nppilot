package main

import (
	"expvar"
	"io"
	"log"
	"net/http"
	"time"
)

import (
	"juju.net.nz/nppilot/rover"
	"juju.net.nz/nppilot/rover/gps"
	"juju.net.nz/nppilot/rover/link"
	"juju.net.nz/nppilot/rover/button"
	"github.com/tarm/goserial"
	"github.com/jessevdk/go-flags"
)

type HeadingOptions struct {
	Kp float32 `long:"heading_kp" default:"4"`
	Ki float32 `long:"heading_ki" default:"0.0"`
	Kd float32 `long:"heading_kd"`
	UMax float32 `long:"heading_umax" default:"0.5"`
	UMin float32 `long:"heading_umin" default:"-0.5"`
	TiLimit float32 `long:"heading_tilimit" default:"0.2"`
	Deadband float32 `long:"heading_deadband"`
}

type SpeedOptions struct {
	Kp float32 `long:"speed_kp" default:"0.05"`
	Ki float32 `long:"speed_ki" default:"0.01"`
	Kd float32 `long:"speed_kd"`
	UMax float32 `long:"speed_umax" default:"1.0"`
	UMin float32 `long:"speed_umin" default:"0.0"`
	TiLimit float32 `long:"speed_tilimit" default:"0.3"`
	Deadband float32 `long:"speed_deadband" default:"0.11"`
}

type DistanceOptions struct {
	Kp float32 `long:"distance_kp" default:"1"`
	Ki float32 `long:"distance_ki"`
	Kd float32 `long:"distance_kd"`
	UMax float32 `long:"distance_umax" default:"6"` // 22 km/h
	UMin float32 `long:"distance_umin" default:"3"` // 11 km/h
	TiLimit float32 `long:"distance_tilimit"`
	Deadband float32 `long:"distance_deadband"`

	TargetSize float32 `long:"target_size" default:"2"`
}

type Options struct {
	GPSPort string `long:"gps_port" default:"/dev/ttyO1"`
	LinkPort string `long:"link_port" default:"/dev/ttyO4"`
	HTTPServer string `long:"http_server" default:":8080"`
	StubPorts bool `long:"stub_ports" default:"false"`
	Controller string `long:"controller" default:"heading"`

	SteeringOffset float32 `long:"steering_offset" default:"0"`
	Heading HeadingOptions `group:"Heading"`
	Speed SpeedOptions `group:"Speed"`
	Distance DistanceOptions `group:"Distance"`
}

type StubReadWriter struct {
}

func (rw *StubReadWriter) Read(p []byte) (n int, err error) {
	time.Sleep(time.Second)
	return 0, nil
}

func (rw *StubReadWriter) Write(p []byte) (n int, err error) {
	return len(p), nil
}

func openPort(name string, baud int) io.ReadWriter {
	c := &serial.Config{Name: name, Baud: baud}
	port, err := serial.OpenPort(c)

	if err == nil {
		return port
	} else if options.StubPorts {
		return &StubReadWriter{}
	} else {
		log.Fatal(err)
		return &StubReadWriter{}
	}
}

var options Options
var parser = flags.NewParser(&options, flags.Default)

func main() {
	if _, err := parser.Parse(); err != nil {
                log.Fatal("invalid arguments.")
        }

	gps := gps.New()
	expvar.Publish("gps", expvar.Func(func() interface{} { return *gps }))

	link := link.New(openPort(options.LinkPort, 57600))
	expvar.Publish("link", expvar.Func(func() interface{} { return link.Stats }))

	headingPID := &rover.PID{
		Kp: options.Heading.Kp,
		Ki: options.Heading.Ki,
		Kd: options.Heading.Kd,
		UMax: options.Heading.UMax,
		UMin: options.Heading.UMin,
		TiLimit: options.Heading.TiLimit,
		Deadband: options.Heading.Deadband,
	}
	speedPID := &rover.PID{
		Kp: options.Speed.Kp,
		Ki: options.Speed.Ki,
		Kd: options.Speed.Kd,
		UMax: options.Speed.UMax,
		UMin: options.Speed.UMin,
		TiLimit: options.Speed.TiLimit,
		Deadband: options.Speed.Deadband,
	}
	distancePID := &rover.PID{
		Kp: options.Distance.Kp,
		Ki: options.Distance.Ki,
		Kd: options.Distance.Kd,
		UMax: options.Distance.UMax,
		UMin: options.Distance.UMin,
		TiLimit: options.Distance.TiLimit,
		Deadband: options.Distance.Deadband,
	}

	recorder := rover.NewRecorder()

	var controller rover.Controller

	switch options.Controller {
	case "sysident":
		controller = &rover.SysIdentController{}
	case "speed":
		controller = &rover.SpeedController{PID: speedPID}
	case "heading":
		controller = &rover.HeadingController{PID: headingPID}
	case "waypoint":
		controller = &rover.WaypointController{
			Heading: headingPID,
			Speed: speedPID,
			Distance: distancePID,
			TargetSize: options.Distance.TargetSize,
		}
	default:
		
	}
	expvar.Publish("controller", expvar.Func(func() interface{} { return controller }))

	driver := rover.New()
	driver.GPS = gps
	driver.Link = link
	driver.Controller = controller
	driver.Switch = button.New()
	driver.SteeringOffset = options.SteeringOffset
	driver.Status.Recorder = recorder

	go driver.Switch.Watch()

	expvar.Publish("state", expvar.Func(func() interface{} { return driver.Status }))

	go gps.Watch(openPort(options.GPSPort, 57600))
	go link.Watch()
	go http.ListenAndServe(options.HTTPServer, nil)
	driver.Run()
}
