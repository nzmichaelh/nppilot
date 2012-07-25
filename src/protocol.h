#pragma once

namespace Protocol {

struct Inputs {
    uint16_t channel[6];
};

struct Heartbeat {
    uint8_t version;
    uint8_t device_id;
};

}
