#include <cxxtest/TestSuite.h>
#include <cstdio>

#include <ringbuffer.h>

class TestRingBuffer : public CxxTest::TestSuite
{
public:
    void test_add()
    {
        RingBuffer<int, 5> ring;

        TS_ASSERT_EQUALS(ring.count(), 0);
        TS_ASSERT_EQUALS(ring.free(), 4);

        TS_ASSERT(ring.add(5));
        TS_ASSERT_EQUALS(ring.count(), 1);
        TS_ASSERT_EQUALS(ring.free(), 3);

        TS_ASSERT(ring.add(6));
        TS_ASSERT(ring.add(7));
        TS_ASSERT_EQUALS(ring.count(), 3);
        TS_ASSERT_EQUALS(ring.free(), 1);

        TS_ASSERT(ring.add(8));
        TS_ASSERT_EQUALS(ring.count(), 4);
        TS_ASSERT_EQUALS(ring.free(), 0);

        TS_ASSERT(!ring.add(9));
        TS_ASSERT_EQUALS(ring.count(), 4);
        TS_ASSERT_EQUALS(ring.free(), 0);
    }

    void test_pop()
    {
        RingBuffer<int, 5> ring;

        TS_ASSERT(ring.add(5));
        TS_ASSERT_EQUALS(ring.pop(), 5);

        TS_ASSERT(ring.add(9));
        TS_ASSERT(ring.add(13));
        TS_ASSERT_EQUALS(ring.pop(), 9);
        TS_ASSERT_EQUALS(ring.pop(), 13);

        TS_ASSERT(ring.add(10));
        TS_ASSERT(ring.add(11));
        TS_ASSERT(ring.add(12));
        TS_ASSERT_EQUALS(ring.pop(), 10);
        TS_ASSERT_EQUALS(ring.pop(), 11);
        TS_ASSERT_EQUALS(ring.pop(), 12);
    }

    void test_add_many()
    {
        RingBuffer<int, 20> ring;

        for (int j = 1; j < 19; j++) {
            for (int i = 0; i < j; i++) {
                TS_ASSERT(ring.add(i+5));
                TS_ASSERT_EQUALS(ring.count(), i+1);
                TS_ASSERT_EQUALS(ring.free(), 20-i-2);
            }

            for (int i = 0; i < j; i++) {
                TS_ASSERT_EQUALS(ring.count(), j-i);
                TS_ASSERT_EQUALS(ring.pop(), i+5);
            }

            TS_ASSERT(ring.empty());
        }
    }

    void test_peek()
    {
        RingBuffer<int, 20> ring;

        ring.extend({11, 12, 13, 14});

        int count;
        const int* p = ring.peek(count);

        TS_ASSERT_EQUALS(count, 4);
        TS_ASSERT_EQUALS(p[0], 11);
        TS_ASSERT_EQUALS(p[1], 12);
        TS_ASSERT_EQUALS(p[2], 13);
        TS_ASSERT_EQUALS(p[3], 14);

        ring.discard(3);
        p = ring.peek(count);
        TS_ASSERT_EQUALS(count, 1);
        TS_ASSERT_EQUALS(p[0], 14);
    }

    void test_peek_wrap()
    {
        RingBuffer<int, 5> ring;
        int count;
        const int* p;

        ring.extend({12, 13, 14});

        p = ring.peek(count);
        TS_ASSERT_EQUALS(count, 3);
        ring.discard(3);

        p = ring.peek(count);
        TS_ASSERT_EQUALS(count, 0);

        ring.extend({12, 13, 14, 15});

        p = ring.peek(count);
        TS_ASSERT_EQUALS(count, 2);
        TS_ASSERT_EQUALS(p[0], 12);
        TS_ASSERT_EQUALS(p[1], 13);
        ring.discard(2);

        p = ring.peek(count);
        TS_ASSERT_EQUALS(count, 2);
        TS_ASSERT_EQUALS(p[0], 14);
        TS_ASSERT_EQUALS(p[1], 15);
        ring.discard(2);
    }
};
