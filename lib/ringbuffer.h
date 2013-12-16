/*
 * Thread safe ring buffer.
 */

#pragma once

/**
 * Ring buffer.  Thread safe if there's one producer and one
 * consumer.  Holds N-1 items.  Done inline as a template as most of
 * these reduce to something very small.
 */
template<typename T, int N>
class RingBuffer
{
public:
    RingBuffer()
        : head_(0), tail_(0)
    {
    }

    /**
     * Add a single item.
     *
     * @return true if the item was added.
     */
    bool add(T item)
    {
        uint8_t next = wrap(head_ + 1);

        if (next != tail_) {
            buffer_[head_] = item;
            head_ = next;
            return true;
        } else {
            return false;
        }
    }

    /**
     * Add a set of items.  Adds as many as possible, discarding the
     * rest.
     */
    void extend(const T* items, int count)
    {
        for (int i = 0; i < count; i++) {
            add(*items++);
        }
    }

#if HAS_INITIALIZER_LIST
    /** Add a set of items.  See extend(). */
    void extend(std::initializer_list<T> list)
    {
        for (T item : list) {
            add(item);
        }
    }
#endif

    /**
     * Get a pointer to the first available item.  Returns the first
     * item and the number of contigious items available past that.
     */
    const T* peek(int& available)
    {
        if (head_ >= tail_) {
            available = head_ - tail_;
        } else {
            available = N - tail_;
        }

        return buffer_ + tail_;
    }

    /**
     * Discard items that are in the buffer.  Discards no more than
     * are buffered.
     */
    void discard(int count)
    {
        if (count >= this->count()) {
            tail_ = head_;
        } else {
            int next = tail_ + count;
            next = (next >= N ? next - N : next);
            tail_ = (uint8_t)next;
        }
    }

    /** Pop the first item from the buffer. */
    T pop()
    {
        if (tail_ != head_) {
            T ch = buffer_[tail_];
            tail_ = wrap(tail_ + 1);
            return ch;
        } else {
            return 0;
        }
    }

    /** Returns the maximum number of items the buffer can hold. */
    int capacity() const { return N-1; }

    /** Returns the number of items in the buffer. */
    int count() const
    {
        int delta = head_ - tail_;
        return delta >= 0 ? delta : delta + N;
    }

    /** Returns the number of free slots in the buffer. */
    int free() const { return N-1-count(); }

    /** Returns true if the buffer is empty. */
    bool empty() const { return head_ == tail_; }

private:
    uint8_t wrap(uint8_t v) const { return v >= N ? 0 : v; }

    T buffer_[N];
    uint8_t head_;
    uint8_t tail_;
};
