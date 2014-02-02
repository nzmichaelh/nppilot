APP = rover
PKG = juju.net.nz/nppilot/$(APP)
EXTERNAL = \
	github.com/tarm/goserial \
	github.com/golang/glog \
	github.com/jessevdk/go-flags

SRC = \
	$(wildcard src/$(PKG)/*/*.go) \
	$(wildcard src/$(PKG)/*.go) \
	$(wildcard *.go)

GOFLAGS = -v -compiler gccgo --gccgoflags \"-static-libgo\"

GOPATH = $(shell pwd)
export GOPATH

all: $(APP)

$(APP): $(SRC) $(EXTERNAL:%=src/%)
	go build $(GOFLAGS) $(APP).go

test:
	go test -v juju.net.nz/...

push: $(APP)
	scp $< blue:~

run: push
	ssh -t blue ./$(APP)

clean:
	rm -rf $(APP) pkg

src/%:
	go get $*
