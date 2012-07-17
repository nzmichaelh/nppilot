import serial
import sys

port = sys.argv[1]

serial = serial.Serial(port, 115200)

serial.setDTR(1)
serial.setDTR(0)
serial.write('1EAF')

