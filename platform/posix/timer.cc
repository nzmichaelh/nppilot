#include "timer.h"
#include "debug.h"

struct Binding;

struct Binding
{
    Timer* ptimer;
    const Timer::Fixed* pfixed;
};

Binding _bindings[50];

void Timer::tick_all()
{
    for (Binding* pbinding = _bindings; pbinding->ptimer != nullptr; pbinding++) {
        pbinding->ptimer->tick(*pbinding->pfixed);
    }
}

void Timer::bind(Timer& timer, const Timer::Fixed& fixed)
{
    for (Binding* pbinding = _bindings; true; pbinding++) {
        if (pbinding->ptimer == nullptr) {
            pbinding->ptimer = &timer;
            pbinding->pfixed = &fixed;
            break;
        }
    }
}
