#pragma once

namespace Protocol {

struct Inputs {
    static const uint16_t Missing = 0;
    static const uint16_t Minimum = 10000;
    static const uint16_t Mid = 15000;
    static const uint16_t Maximum = 20000;

    uint16_t channel[6];
};

struct Heartbeat {
    uint8_t version;
    uint8_t device_id;
};

struct Drives {
    static const uint16_t NotControlled = 1;
    static const uint16_t Minimum = 10000;
    static const uint16_t Mid = 15000;
    static const uint16_t Maximum = 20000;

    uint16_t channel[6];
    bool armed;
};

}
