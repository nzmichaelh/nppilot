#pragma once

namespace Protocol {

enum class Code : uint8_t {
    Heartbeat = 'h',
    Pong = 'p',
    Inputs = 'i',
    Request = 'R',
    Version = 'v',
};

struct Generic {
};

struct Heartbeat {
    Code code;
    uint8_t version;
    uint8_t device_id;
    uint8_t ticks;
    uint8_t state;
};

struct Pong {
    Code code;
};

struct Inputs {
    static const uint8_t Missing = 0;

    Code code;
    uint8_t channels[6];
};

struct Servo {
    static const uint16_t NotControlled = 1;
    static const uint16_t Minimum = 10000;
    static const uint16_t Mid = 15000;
    static const uint16_t Maximum = 20000;

    Code code;
    int8_t channel[6];
    bool armed;
};

struct Request {
    Code code;
    Code requested;
};

struct Version {
    Code code;
    char version[18];
};

}  // namespace Protocol
