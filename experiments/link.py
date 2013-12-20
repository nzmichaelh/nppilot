import serial
import time


def checksum(values):
    sum1 = 0x12
    sum2 = 0x34

    for value in values:
        sum1 += value
        if sum1 >= 255: sum1 -= 255
        sum2 += sum1
        if sum2 >= 255: sum2 -= 255

    return (sum1 ^ sum2) & 0xFF

        
def main():
    port = serial.Serial('/dev/ttyUSB0', 38400, timeout=0.1)

    Mark = ord('\n')
    Escape = ord('^')
    Xor = 0x20

    rx = []
    xor = 0
    start = time.time()

    while True:
        got = port.read(1)
        if got:
            ch = ord(got)

            if ch == Mark:
                if len(rx) >= 2:
                    body = rx[:-1]
                    if checksum(body) == rx[-1]:
                        print('<<', chr(body[0]), repr(body))
                    else:
                        print('Checksum failure.')
                        assert False
                rx = []
                every = []
            elif ch == Escape:
                xor = Xor
            else:
                rx.append(ch ^ xor)
                xor = 0

        now = time.time()
        elapsed = now - start

        if elapsed >= 0.05:
            start = now
            tx = [ord('p')]
            tx.append(checksum(tx))
            tx.append(Mark)
            print('>>', chr(tx[0]), tx)
            port.write(bytes(tx))


if __name__ == '__main__':
    main()
