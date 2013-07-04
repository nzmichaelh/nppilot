#include <cstdint>
#include <algorithm>
#include <cstring>

#include "usblink.h"

USBLink::USBLink()
    : tx_()
{
}

void USBLink::write(const uint8_t* data, int length)
{
}

bool USBLink::flush()
{
    return true;
}
