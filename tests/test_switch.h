#include <switch.h>
#include <cxxtest/TestSuite.h>

class TestSwitch : public CxxTest::TestSuite
{
public:
    void test_exact()
    {
        Switch aswitch;
        Switch::Fixed fixed = { { 1000, 2000, 3000 } };

        TS_ASSERT_EQUALS(aswitch.update(fixed, 1000), 0);
        TS_ASSERT_EQUALS(aswitch.update(fixed, 2000), 1);
        TS_ASSERT_EQUALS(aswitch.update(fixed, 3000), 2);
    }

    void test_close()
    {
        Switch aswitch;
        Switch::Fixed fixed = { { 1000, 2000, 3000 } };

        TS_ASSERT_EQUALS(aswitch.update(fixed, 1020), 0);
        TS_ASSERT_EQUALS(aswitch.update(fixed, 980), 0);
        TS_ASSERT_EQUALS(aswitch.update(fixed, 2020), 1);
        TS_ASSERT_EQUALS(aswitch.update(fixed, 1980), 1);
        TS_ASSERT_EQUALS(aswitch.update(fixed, 3020), 2);
        TS_ASSERT_EQUALS(aswitch.update(fixed, 2980), 2);
    }

    void test_middle()
    {
        Switch aswitch;
        Switch::Fixed fixed = { { 1000, 2000, 3000 } };

        TS_ASSERT_EQUALS(aswitch.update(fixed, 1499), 0);
        TS_ASSERT_EQUALS(aswitch.update(fixed, 2499), 1);
    }

    void test_hysteresis()
    {
        Switch aswitch;
        Switch::Fixed fixed = { { 1000, 2000, 3000 } };

        TS_ASSERT_EQUALS(aswitch.update(fixed, 2000), 1);
        /* Middle stays at position 1 */
        TS_ASSERT_EQUALS(aswitch.update(fixed, 1510), 1);
        TS_ASSERT_EQUALS(aswitch.update(fixed, 1500), 1);
        TS_ASSERT_EQUALS(aswitch.update(fixed, 1490), 1);

        /* Move a fair way before it triggers */
        TS_ASSERT_EQUALS(aswitch.update(fixed, 1401), 1);
        TS_ASSERT_EQUALS(aswitch.update(fixed, 1399), 0);
    }
};
