#include <debug.h>
#include <cassert>
#include <cstdarg>

void Debug::info(const char* pmsg, ...)
{
    print("info: ");

    va_list args;
    va_start(args, pmsg);
    vprint(pmsg, args);
    print("\r\n");
}

void Debug::error(const char* pmsg, ...)
{
    print("error: ");

    va_list args;
    va_start(args, pmsg);
    vprint(pmsg, args);
    print("\r\n");
}

void Debug::print(const char* pmsg)
{
    const char* pstop = pmsg + 100;

    for (; *pmsg != 0 && pmsg < pstop; pmsg++) {
        putch(*pmsg);
    }
}

void Debug::vprint(const char* pmsg, va_list args)
{
    const char* pstop = pmsg + 100;

    for (; *pmsg != 0 && pmsg < pstop; pmsg++) {
        char ch = *pmsg;

        if (ch == '%') {
            ch = *++pmsg;

            switch (ch) {
            case '%':
                putch(ch);
                break;
            case 'd':
                printn(va_arg(args, int), 10, true);
                break;
            case 'u':
                printn(va_arg(args, int), 10, false);
                break;
            case 'x':
                printn(va_arg(args, int), 16, false);
                break;
            case 's':
                print(va_arg(args, char *));
                break;
            default:
                assert(false);
                break;
            }
        } else {
            putch(ch);
        }
    }
}

void Debug::printn(int value, unsigned int base, bool is_signed)
{
    static const char digits[] = "0123456789abcdef";

    if (is_signed && value < 0) {
        putch('-');
        value = -value;
    }

    unsigned int v = value;
    char stack[11];
    char* p = stack + sizeof(stack);
    *--p = '\0';

    do {
        unsigned int next = v / base;
        int remainder = v - next * base;
        *--p = digits[remainder];
        v = next;
    } while (v != 0);

    print(p);
}
