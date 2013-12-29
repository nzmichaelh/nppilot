#pragma once

namespace Protocol {

enum class Code : uint8_t {
    Heartbeat = 'h',
    Pong = 'p',
    Inputs = 'i',
    Request = 'R',
    Version = 'v',
    Demand = 'd',
    State = 's',
};

struct Generic {
};

struct Heartbeat {
    Code code;

    uint8_t version;
    uint8_t device_id;
    uint8_t ticks;
};

struct State {
    enum class Flags : uint8_t {
        None = 0,
        InControl = 1,
    };

    Code code;
    Flags flags;
};

struct Pong {
    Code code;
};

struct Input {
    static const int8_t Missing = -128;

    Code code;
    int8_t channels[6];
};

struct Demand {
    enum class Flags : uint8_t {
        None = 0,
        TakeControl = 1,
    };

    Code code;
    Flags flags;
    int8_t channels[6];
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
