class HAL
{
public:
    static void init();
    static void start();
    static void poll();
    static void wait();

    static void set_status_led(bool on);
    static void flash();
};
