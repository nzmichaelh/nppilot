#include <cxxtest/TestSuite.h>
#include <vector>

#include <switcher.h>

static std::vector<int> order;

static void one()
{
    order.push_back(1);
    order.push_back(101);
}

static void four()
{
    order.push_back(4);
    order.push_back(104);
}

class TestSwitcher : public CxxTest::TestSuite
{
public:
    void setUp()
    {
        order.clear();
    }

    void test_idle()
    {
        Switcher switcher;

        for (int i = 0; i < 20; i++) {
            switcher.next();
        }

        TS_ASSERT_EQUALS(order.size(), 0);
    }

    void test_one()
    {
        Switcher switcher;

        for (int i = 0; i < 5; i++) {
            switcher.next();
        }

        switcher.trigger(1);

        for (int i = 0; i < 5; i++) {
            switcher.next();
        }

        static const int expect[] = { 1, 101, -1 };
        TS_ASSERT_EQUALS(matches(expect), OK);
    }

    void test_retrigger()
    {
        Switcher switcher;

        switcher.next();

        switcher.trigger(1);
        switcher.next();

        order.push_back(99);

        switcher.next();
        switcher.trigger(1);
        switcher.next();

        static const int expect[] = { 1, 101, 99, 1, 101, -1 };
        TS_ASSERT_EQUALS(matches(expect), OK);
    }

    void test_priority()
    {
        Switcher switcher;

        switcher.next();

        switcher.trigger(1);
        switcher.trigger(4);

        switcher.next();

        static const int expect[] = { 4, 104, 1, 101, -1 };
        TS_ASSERT_EQUALS(matches(expect), OK);
    }

    void test_priority_2()
    {
        Switcher switcher;

        switcher.next();

        switcher.trigger(4);
        switcher.trigger(1);

        switcher.next();

        static const int expect[] = { 4, 104, 1, 101, -1 };
        TS_ASSERT_EQUALS(matches(expect), OK);
    }

private:
    enum Match
    {
        OK,
        TooLong,
        TooShort,
        Mismatch,
    };

    Match matches(const int* p)
    {
        for (int& x : order) {
            if (*p == -1) {
                return TooLong;
            }
            else if (*p != x) {
                return Mismatch;
            }

            p++;
        }

        if (*p != -1) {
            return TooShort;
        }

        return OK;
    }
};

const Switcher::thread_entry Switcher::_dispatch[] =
{
    nullptr,
    one,
    nullptr,
    nullptr,
    four,
};
