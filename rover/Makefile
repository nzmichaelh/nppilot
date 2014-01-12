APP = rover
PKG = juju.net.nz/nppilot/$(APP)
EXTERNAL = github.com/tarm/goserial

SRC = \
	$(wildcard src/$(PKG)/*/*.go) \
	$(wildcard src/$(PKG)/*.go) \
	$(wildcard *.go)

GOPATH = $(shell pwd)
export GOPATH

all: $(APP)

$(APP): $(SRC) $(EXTERNAL:%=src/%)
	go build $(APP).go

push: $(APP)
	scp $< blue:~

run: push
	ssh -t blue ./$<

clean:
	rm -rf $(APP) $(EXTERNAL:%=src/%)

src/%:
	go get $*
