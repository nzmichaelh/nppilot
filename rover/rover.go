package main

import (
	"expvar"
	"io"
	"log"
	"net/http"
	"time"
	"os"
)

import (
	"juju.net.nz/config"
	"juju.net.nz/nppilot/rover"
	"juju.net.nz/nppilot/rover/gps"
	"juju.net.nz/nppilot/rover/link"
	"juju.net.nz/nppilot/rover/button"
	"github.com/tarm/goserial"
	"github.com/golang/glog"
)

type Options struct {
	GpsPort string
	LinkPort string
	HttpServer string
	StubPorts bool
	Controller string

	SteeringOffset float32
	TargetSize float32
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

func openPort(name string, baud int, stub bool) io.ReadWriter {
	c := &serial.Config{Name: name, Baud: baud}
	port, err := serial.OpenPort(c)

	if err == nil {
		return port
	} else if stub {
		return &StubReadWriter{}
	} else {
		log.Fatal(err)
		return &StubReadWriter{}
	}
}

func main() {
	config := &config.Config{}

	options := &Options{
		GpsPort: "/dev/ttyO1",
		LinkPort: "/dev/ttyO4",
		HttpServer: ":8080",
		StubPorts: false,
		Controller: "heading",
		TargetSize: 2,
		SteeringOffset: -0.1,
	}
	config.Add("Options", "", options)

	headingPID := &rover.PID{
		Kp: 1,
		UMax: 0.3,
		UMin: -0.3,
		TiLimit: 0.2,
	}
	config.Add("Heading", "heading", headingPID)

	speedPID := &rover.PID{
		Kp: 0.05,
		Ki: 0.01,
		UMax: 1.0,
		UMin: 0.0,
		TiLimit: 0.3,
		Deadband: 0.11,
	}
	config.Add("Speed", "speed", speedPID)

	distancePID := &rover.PID{
		Kp: 1,
		UMax: 6,
		UMin: 3,
	}
	config.Add("Distance", "distance", distancePID)

	logConfig := &glog.Config{Verbosity: 2, Compress: true}
	config.Add("Logging", "log", logConfig)

	if errors := config.ParseArgv(); len(errors) > 0 {
		config.Help(nil)
		config.ShowErrors(nil, errors)
		os.Exit(1)
	}

	expvar.Publish("config", config)
	glog.Init(logConfig)

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
			TargetSize: options.TargetSize,
		}
	}
	expvar.Publish("controller", expvar.Func(func() interface{} { return controller }))

	gps := gps.New()
	expvar.Publish("gps", expvar.Func(func() interface{} { return &gps.Stats }))

	link := link.New(openPort(options.LinkPort, 57600, options.StubPorts))
	expvar.Publish("link", expvar.Func(func() interface{} { return link.Stats }))


	driver := rover.New()
	driver.GPS = gps
	driver.Link = link
	driver.Controller = controller
	driver.Switch = button.New()
	driver.SteeringOffset = options.SteeringOffset
	driver.Status.Recorder = recorder

	go driver.Switch.Watch()

	expvar.Publish("state", expvar.Func(func() interface{} { return driver.Status }))

	go gps.Watch(openPort(options.GpsPort, 57600, options.StubPorts))
	go link.Watch()
	go http.ListenAndServe(options.HttpServer, nil)
	driver.Run()
}
