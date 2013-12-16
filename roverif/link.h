class Link
{
public:
    Link();

    /** Send a frame */
    void send(int id, const void* msg, int length);

    /**
     * Feed received data, dispatching received messages as they're
     * decoded.
     */
    void feed(const uint8_t* data, int length);
};
