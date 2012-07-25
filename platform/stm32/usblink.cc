#include <cstdint>
#include <algorithm>
#include <cstring>

#include "usblink.h"

#include "libmaple/usb_cdcacm.h"
#include "libmaple/usart.h"
#include "libmaple/systick.h"

USBLink::USBLink()
    : tx_()
{
}

void USBLink::write(const uint8_t* data, int length)
{
    tx_.extend(data, length);
}

bool USBLink::flush()
{
    if (!tx_.empty()) {
        int available;
        const uint8_t* p = tx_.peek(available);

        int wrote = usb_cdcacm_tx(p, available);

        if (wrote > 0) {
            usart_putudec(USART2, wrote);
            usart_putstr(USART2, "  ");
            tx_.discard(wrote);
        }
    }
}
