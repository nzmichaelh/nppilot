package main

import (
	"expvar"
	"flag"
	"io"
	"log"
	"net/http"
	"time"
)

import (
	"juju.net.nz/nppilot/rover"
	"juju.net.nz/nppilot/rover/gps"
	"juju.net.nz/nppilot/rover/link"
	"github.com/tarm/goserial"
)

var gpsPort = flag.String("gps_port", "/dev/ttyO1", "GPS port.")
var linkPort = flag.String("link_port", "/dev/ttyO4", "Link port.")
var httpServer = flag.String("http_server", ":8080", "HTTP server address.")
var stubPorts = flag.Bool("stub_ports", false, "Stub out missing serial ports.")
var controllerName = flag.String("controller", "speed", "Controller to run.")
var kp = flag.Float64("kp", 0.05, "Proportional gain.")
var ki = flag.Float64("ki", 0.01, "Integral gain.")
var umax = flag.Float64("umax", 1.0, "Max controller drive.")
var tilimit = flag.Float64("tilimit", 0.2, "Integral limit.")

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
	} else if *stubPorts {
		return &StubReadWriter{}
	} else {
		log.Fatal(err)
		return &StubReadWriter{}
	}
}

func main() {
	flag.Parse()

	gps := gps.New()
	expvar.Publish("gps", expvar.Func(func() interface{} { return *gps }))

	link := link.New(openPort(*linkPort, 38400))
	expvar.Publish("link", expvar.Func(func() interface{} { return link.Stats }))

	pid := &rover.PID{
		Kp: float32(*kp), Ki: float32(*ki), Kd: 0,
		UMax: float32(*umax), UMin: 0.0,
		TiLimit: float32(*tilimit),
		Deadband: 0.11,
	}

	var controller rover.Controller

	switch *controllerName {
	case "speed":
		controller = &rover.SpeedController{PID: pid}
	case "sysident":
		controller = &rover.SysIdentController{}
	default:
		
	}
	expvar.Publish("controller", expvar.Func(func() interface{} { return controller }))

	driver := &rover.Driver{GPS: gps, Link: link, Controller: controller}
	expvar.Publish("state", expvar.Func(func() interface{} { return driver.Status }))

	go gps.Watch(openPort(*gpsPort, 57600))
	go link.Watch()
	go http.ListenAndServe(*httpServer, nil)
	driver.Run()
}
