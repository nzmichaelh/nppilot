extern void systick();
void irq_timer4ch1();
void irq_timer3ch2();
void irq_timer3ch3();
void irq_timer3ch4();
void irq_timer2ch1();
void irq_timer2ch2();

void init();
int main();

enum ThreadID {
    BlinkerID,
    SysTickID,
    HeartbeatID,
    PollID,
    SupervisorID,
};
