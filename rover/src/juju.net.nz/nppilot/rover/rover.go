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
	"juju.net.nz/nppilot/rover/gps"
	"juju.net.nz/nppilot/rover/link"
	"github.com/tarm/goserial"
)

var gpsPort = flag.String("gps_port", "/dev/ttyO1", "GPS port.")
var linkPort = flag.String("link_port", "/dev/ttyO4", "Link port.")
var httpServer = flag.String("http_server", ":8080", "HTTP server address.")
var stubPorts = flag.Bool("stub_ports", false, "Stub out missing serial ports.")

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

	var gps gps.Link
	go gps.Watch(openPort(*gpsPort, 57600))

	expvar.Publish("gps", expvar.Func(func() interface{} { return gps }))

	var link link.Link
	go link.Watch(openPort(*linkPort, 38400))

	expvar.Publish("link", expvar.Func(func() interface{} { return link }))

	log.Fatal(http.ListenAndServe(*httpServer, nil))
}
