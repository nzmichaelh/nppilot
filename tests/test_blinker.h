#include <blinker.h>

#include <initializer_list>
#include <vector>

#include <cxxtest/TestSuite.h>

class TestBlinker : public CxxTest::TestSuite
{
public:
    void setUp()
    {
        wrote_.clear();
    }

    void test_on_off()
    {
        Blinker blink;
        blink.set(0b101);
        
        for (int i = 0; i < 6; i++) {
            blink.tick();
        }

        TS_ASSERT_EQUALS(check({true, false, true, false, true, false}), 0);
    }

    void test_off()
    {
        Blinker blink;
        blink.set(0);
        
        for (int i = 0; i < 6; i++) {
            blink.tick();
        }

        TS_ASSERT_EQUALS(check({false, false, false, false, false, false}), 0);
    }

    void test_on()
    {
        Blinker blink;
        blink.set(0b11);
        
        for (int i = 0; i < 6; i++) {
            blink.tick();
        }

        TS_ASSERT_EQUALS(check({true, true, true, true, true, true}), 0);
    }

    void test_preiod()
    {
        Blinker blink;

        blink.set(0b100001);
        
        for (int i = 0; i < 11; i++) {
            blink.tick();
        }

        TS_ASSERT_EQUALS(check({true, false, false, false, false,
                        true, false, false, false, false,
                        true}), 0);
    }

private:
    friend class Blinker;

    int check(std::initializer_list<bool> list)
    {
        if (list.size() < wrote_.size()) {
            return -1;
        } else if (list.size() > wrote_.size()) {
            return -2;
        } else {
            int i = 0;

            for (bool j : list) {
                if (wrote_[i] != j) {
                    return i+1;
                }
                i++;
            }

            return 0;
        }
    }

    static std::vector<bool> wrote_;
};

std::vector<bool> TestBlinker::wrote_;

void Blinker::update(bool value)
{
    TestBlinker::wrote_.push_back(value);
}
