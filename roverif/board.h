
struct Board
{
    static const int AHBClock = 72000000;
    static const int APB1Clock = AHBClock / 2;
    static const int APB2Clock = AHBClock / 1;
    static const int LSIClock = 40000;
    static const int DebugBaudRate = 115200;

    static const int Ticks = 1000;
};
