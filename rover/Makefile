APP = rover
PKG = juju.net.nz/nppilot/$(APP)
EXTERNAL = \
	github.com/tarm/goserial \
	github.com/golang/glog

SRC = \
	$(wildcard src/$(PKG)/*/*.go) \
	$(wildcard src/$(PKG)/*.go) \
	$(wildcard src/juju.net.nz/*/*.go) \
	$(wildcard *.go)

GOFLAGS = -v -compiler gccgo
#GOFLAGS += --gccgoflags \"-static-libgo\"

GOPATH = $(shell pwd)
export GOPATH

all: $(APP)

$(APP): $(SRC) $(EXTERNAL:%=src/%)
	go build $(GOFLAGS) $(APP).go

test:
	go test -v juju.net.nz/...

push: $(APP)
	rsync --size-only -l blue:/tmp/rover.* ~/log
	rsync -zt $< blue:~

run: push
	ssh -t blue ./$(APP)

clean:
	rm -rf $(APP) pkg

src/github.com/golang/glog:
	mkdir -p $(@D)
	cd $(@D) && git clone juju:~/p/git/glog

src/%:
	go get $*
	cat /dev/null $(wildcard patches/$(notdir $*)*.patch) | patch -p1 -d src/$*
