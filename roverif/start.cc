#include "roverif.h"
#include <avr/interrupt.h>

int main()
{
    RoverIf::init();
    sei();
    RoverIf::run();

    return 0;
}
