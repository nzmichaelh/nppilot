#include <cxxtest/TestSuite.h>
#include <vector>
#include <cstdio>

#include <cobs.h>

struct Dispatched {
    int id;
    uint8_t msg[256];
    int length;
};

static std::vector<uint8_t> wrote;
static std::vector<Dispatched> dispatched;

void COBSLink::write(const uint8_t* p, int length)
{
    for (int i = 0; i < length; i++) {
        wrote.push_back(*p++);
    }
}

void COBSLink::dispatch(int id, const void* msg, int length)
{
    Dispatched record = { .id = id, .msg = {}, .length = length };
    memcpy(record.msg, msg, length);
    dispatched.push_back(record);
}

class TestCOBSLink : public CxxTest::TestSuite
{
public:
    void setUp()
    {
        wrote.clear();
        dispatched.clear();
    }

    void test_send()
    {
        COBSLink link;
        link.send('i', "Hi there", 8);
    }

    void test_roundtrip()
    {
        COBSLink link;
        Dispatched got;

        TS_ASSERT_EQUALS(round_trip(link, 'i', "Hi there", 8, got), 0);
        TS_ASSERT_EQUALS(round_trip(link, 'i', "Hi\nthere!\n", 11, got), 0);

        /* Just a \n */
        TS_ASSERT_EQUALS(round_trip(link, 'i', "\n", 1, got), 0);
        TS_ASSERT_EQUALS(round_trip(link, 'i', "\n\n\n", 1, got), 0);

        /* Final \n */
        TS_ASSERT_EQUALS(round_trip(link, 'i', "GPS\n", 1, got), 0);
    }

    void test_empty()
    {
        COBSLink link;
        Dispatched got;

        /* Empty message */
        TS_ASSERT_EQUALS(round_trip(link, 'i', "", 0, got), 0);
    }

    void test_all_characters()
    {
        COBSLink link;
        Dispatched got;

        for (int i = 0; i <= 255; i++) {
            uint8_t send[] = { (uint8_t)i, (uint8_t)(i+1), (uint8_t)(i+2) };
            TS_ASSERT_EQUALS(round_trip(link, 'i', send, sizeof(send), got), 0);
        }
    }

private:
    int round_trip(COBSLink& link, int id, const void* msg, int length, Dispatched& got)
    {
        link.send(id, msg, length);

        for (uint8_t& ch : wrote) {
            link.feed(&ch, 1);
        }

        wrote.clear();

        if (dispatched.size() != 1) {
            return 1;
        } else {
            got = dispatched.back();
            dispatched.pop_back();

            /* Keep them separate for debugging */
            if (got.id != id) {
                return 2;
            } else if (got.length != length) {
                return 3;
            } else if (memcmp(got.msg, msg, length) != 0) {
                return 4;
            } else {
                return 0;
            }
        }
    }
};
