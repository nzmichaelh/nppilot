#pragma once

#include <bitarray.h>

class Switcher
{
public:
    Switcher();

    void trigger(int id);
    void next();

private:
    typedef void (*thread_entry)(void);

    BitArray _running;
    BitArray _pending;

    static const thread_entry _dispatch[];
};
