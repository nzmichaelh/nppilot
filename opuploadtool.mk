#
# Builds the command line OpenPilot programmer
#
# A hack.  Copy to ground/openpilotgcs/src/experimental/USB_UPLOAD_TOOL
# and run with `make -f opuploadtool.mk`
#
# Michael Hope <michaelh@juju.net.nz>
#

SSP_DIR = SSP
SSP_SRC = port.cpp qssp.cpp qsspt.cpp

RAWHID_DIR = ../../plugins/rawhid
RAWHID_SRC = pjrc_rawhid_unix.cpp usbmonitor_linux.cpp musbmonitor.cpp mpjrc_rawhid.cpp

QEXTSERIALPORT_DIR = ../../libs/qextserialport/src
QEXTSERIALPORT_SRC = qextserialport.cpp qextserialenumerator_unix.cpp posix_qextserialport.cpp mqextserialport.cpp mqextserialenumerator.cpp

SRC = main.cpp op_dfu.cpp

CXXFLAGS = $(shell pkg-config --libs --cflags QtCore) -I.
CXXFLAGS += -DQ_OS_UNIX -DRAWHID_LIBRARY
CXXFLAGS += -O2

ALL_SRC = \
	$(SSP_SRC:%=$(SSP_DIR)/%) \
	$(RAWHID_SRC:%=$(RAWHID_DIR)/%) \
	$(QEXTSERIALPORT_SRC:%=$(QEXTSERIALPORT_DIR)/%) \
	$(SRC:%=./%)

ALL_OBJ = $(ALL_SRC:cpp=o)

opuploadtool: $(ALL_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(shell pkg-config --libs QtCore) -lusb -ludev

m%.cpp: %.h
	moc -o $@ $<

clean:
	rm -f $(ALL_OBJ)
