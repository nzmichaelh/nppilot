#pragma once

#include <cstdint>
#include "protocol.h"
#include <MPU6050/MPU6050.h>

class IMU {
public:
    IMU();

    void init();
    bool read(Protocol::IMU* pinto);
    void check();

private:
    friend class I2Cdev;

    static uint8_t wait();

    static int8_t read_bytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);
    static int8_t write_byte(uint8_t devAddr, uint8_t regAddr, uint8_t data);
    static int8_t set_address(uint8_t device, uint8_t address);

    MPU6050 mpu_;
    bool good_;
};

