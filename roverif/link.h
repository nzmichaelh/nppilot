class Link
{
public:
    Link();

    /** Send a frame */
    void* start();
    void send(uint8_t length);

    /**
     * Feed received data, dispatching received messages as they're
     * decoded.
     */
    void feed(const uint8_t* data, int length);

private:
    uint8_t tx_[8];
    uint8_t tx_at_;
    uint8_t tx_end_;
};
