#include <debug.h>
#include <cxxtest/TestSuite.h>

class TestDebug : public CxxTest::TestSuite
{
public:
    void setUp()
    {
        _at = 0;
    }

    void test_basic()
    {
        Debug::info("rock on!");
        TS_ASSERT_EQUALS(0, strcmp("info: rock on!\r\n", _msg));
    }

    void test_signed()
    {
        Debug::info("decimal %d.", -1234);
        TS_ASSERT_EQUALS(0, strcmp("info: decimal -1234.\r\n", _msg));
    }

    void test_unsigned()
    {
        Debug::info("unsigned %u.", -1234);
        TS_ASSERT_EQUALS(0, strcmp("info: unsigned 4294966062.\r\n", _msg));
    }

    void test_hex()
    {
        Debug::info("hex %x.", 0xabcd5678);
        TS_ASSERT_EQUALS(0, strcmp("info: hex abcd5678.\r\n", _msg));
    }

    void test_string()
    {
        Debug::info("str %s.", "rock on!");
        TS_ASSERT_EQUALS(0, strcmp("info: str rock on!.\r\n", _msg));
    }

    static int _at;
    static char _msg[128];
};

void Debug::putch(char ch)
{
    TestDebug::_msg[TestDebug::_at++] = ch;
    TestDebug::_msg[TestDebug::_at] = '\0';
}

#ifdef CXXTEST_RUNNING
int TestDebug::_at;
char TestDebug::_msg[128];
#endif
