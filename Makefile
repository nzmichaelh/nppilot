SUBDIRS = roverif rover

all: $(SUBDIRS:%=build-%)

clean: $(SUBDIRS:%=clean-%)

build-%:
	$(MAKE) -C $*

clean-%:
	$(MAKE) -C $* clean

