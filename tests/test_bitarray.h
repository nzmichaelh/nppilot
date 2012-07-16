#include <bitarray.h>
#include <cxxtest/TestSuite.h>

class TestBitArray : public CxxTest::TestSuite
{
public:
    void test_set_get()
    {
        BitArray b;

        TS_ASSERT_EQUALS(b.item(2), false);
        TS_ASSERT_EQUALS(b.item(3), false);

        b.set(2, true);
        TS_ASSERT_EQUALS(b.item(2), true);
        TS_ASSERT_EQUALS(b.item(3), false);

        b.set(3, true);
        TS_ASSERT_EQUALS(b.item(2), true);
        TS_ASSERT_EQUALS(b.item(3), true);

        TS_ASSERT_EQUALS(b.item(31), false);
        b.set(31, true);
        TS_ASSERT_EQUALS(b.item(31), true);
    }

    void test_last_set()
    {
        BitArray b;
        TS_ASSERT_EQUALS(b.last_set(), -1);

        b.set(2, true);
        TS_ASSERT_EQUALS(b.last_set(), 2);

        b.set(30, true);
        TS_ASSERT_EQUALS(b.last_set(), 30);

        b.set(25, true);
        TS_ASSERT_EQUALS(b.last_set(), 30);

        b.set(30, false);
        TS_ASSERT_EQUALS(b.last_set(), 25);
    }
};
