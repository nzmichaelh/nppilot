#include "imu.h"
#include "hal.h"
#include "link.h"

#include <I2Cdev/I2Cdev.h>
#include <avr/io.h>
#include <avr/interrupt.h>


enum Codes {
    /* Master */
    TW_NONE = 0,
    TW_START = 0x08,
    TW_REP_START = 0x10,
    /* Master Transmitter */
    TW_MT_SLA_ACK = 0x18,
    TW_MT_SLA_NACK = 0x20,
    TW_MT_DATA_ACK = 0x28,
    TW_MT_DATA_NACK = 0x30,
    TW_MT_ARB_LOST = 0x38,
    /* Master Receiver */
    TW_MR_ARB_LOST = 0x38,
    TW_MR_SLA_ACK = 0x40,
    TW_MR_SLA_NACK = 0x48,
    TW_MR_DATA_ACK = 0x50,
    TW_MR_DATA_NACK = 0x58,
};

IMU::IMU()
    : mpu_(MPU6050_ADDRESS_AD0_HIGH), good_(false) {
}

void IMU::init() {
    TWCR = 0;
    TWSR = 0;
    TWBR = ((F_CPU / HAL::I2CRate) - 16) / 2;
    TWCR = _BV(TWEN);

    mpu_.initialize();
}

uint8_t IMU::wait() {
    for (uint8_t i = 0; i < 250; i++) {
        if ((TWCR & _BV(TWINT)) != 0) {
            return TWSR & 0xF8;
        }
    }
    return TW_NONE;
}

bool IMU::read(Protocol::IMU* pinto) {
    if (!mpu_.testConnection()) {
        good_ = false;
        return false;
    }

    mpu_.getMotion6(pinto->accels + 0,
                    pinto->accels + 1,
                    pinto->accels + 2,
                    pinto->gyros + 0,
                    pinto->gyros + 1,
                    pinto->gyros + 2);
    good_ = true;
    return true;
}

void IMU::check() {
    if (!good_) {
        init();
    }
}

#define ERRORX(x) { dbg("err " #x "\r\n"); return -x; }
#define ERROR(x) { return -x; }
#define DBG(x)

int8_t IMU::set_address(uint8_t device, uint8_t address) {
    uint8_t twst;
    DBG("set_address ");

    // Send the start/
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);

    twst = wait();
    if (twst != TW_START && twst != TW_REP_START) ERROR(2);

    // Send the device address.
    TWDR = (device << 1) | 0;
    TWCR = _BV(TWINT) | _BV(TWEN);

    if (wait() != TW_MT_SLA_ACK) ERROR(4);

    // Write the address byte.
    TWDR = address;
    TWCR = _BV(TWINT) | _BV(TWEN);
    if (wait() != TW_MT_DATA_ACK) ERROR(6);
    ERROR(0);
}

int8_t IMU::read_bytes(uint8_t device, uint8_t address, uint8_t count, uint8_t *data) {
    DBG("rb ");
    int8_t err;

    if ((err = set_address(device, address)) != 0) {
        return err;
    }

    // Send the start.
    TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTA);
    if (wait() != TW_REP_START) ERROR(8);

    TWDR = (device << 1) | 1;
    TWCR = _BV(TWINT) | _BV(TWEN);
    if (wait() != TW_MR_SLA_ACK) ERROR(10);

    uint8_t i;
    for (i = 0; i < count - 1; i++) {
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
        if (wait() != TW_MR_DATA_ACK) ERROR(13);
        data[i] = TWDR;
    }

    TWCR = _BV(TWINT) | _BV(TWEN);
    if (wait() != TW_MR_DATA_NACK) ERROR(14);
    data[i] = TWDR;

    // Stop.
    TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);

    ERROR(0);
}

int8_t IMU::write_byte(uint8_t device, uint8_t address, uint8_t value) {
    DBG("wb ");

    int8_t err;
    if ((err = set_address(device, address)) != 0) {
        return err;
    }

    TWDR = value; // send data to the previously addressed device
    TWCR = _BV(TWINT) | _BV(TWEN);
    if (wait() != TW_MT_DATA_ACK) ERROR(8);

    // Stop.
    TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);

    ERROR(0);
}

// Thanks Fastwire.h for the examples:
//   https://simplo.googlecode.com/svn/trunk/fastwire.h


uint16_t I2Cdev::readTimeout;

bool I2Cdev::writeBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data) {
    uint8_t b = 0;
    readByte(devAddr, regAddr, &b);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    return writeByte(devAddr, regAddr, b);
}

bool I2Cdev::writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data) {
    uint8_t b = 0;
    if (readByte(devAddr, regAddr, &b) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        data <<= (bitStart - length + 1); // shift data into correct position
        data &= mask; // zero all non-important bits in data
        b &= ~(mask); // zero all important bits in existing byte
        b |= data; // combine data with existing byte
        return writeByte(devAddr, regAddr, b);
    } else {
        return false;
    }
}

int8_t I2Cdev::readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data, uint16_t timeout) {
    uint8_t count, b = 0;
    if ((count = readByte(devAddr, regAddr, &b, timeout)) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        b &= mask;
        b >>= (bitStart - length + 1);
        *data = b;
    }
    return count;
}

int8_t I2Cdev::readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, uint16_t timeout)
{
    return IMU::read_bytes(devAddr, regAddr, length, data) == 0 ? length : -1;
}

int8_t I2Cdev::readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint16_t timeout) {
    return readBytes(devAddr, regAddr, 1, data, timeout);
}

bool I2Cdev::writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
    return IMU::write_byte(devAddr, regAddr, data) == 0;
}
