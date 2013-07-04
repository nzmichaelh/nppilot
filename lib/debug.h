/*
 * Debug message and assertion support.
 */

#pragma once

#include <cstdarg>

class Debug
{
public:
    static void info(const char* pmsg, ...);
    static void error(const char* pmsg, ...);

private:
    static void print(const char* pmsg);
    static void printn(int value, unsigned int base, bool is_signed);
    static void vprint(const char* pmsg, va_list args);

    static void putch(char ch);
};
