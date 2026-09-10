#pragma once

namespace evaf
{
    /**
     * @brief Fixed-capacity circular ring buffer.
     * @tparam T Element type
     * @tparam N Buffer capacity
     */
    template <typename T, unsigned char N>
    class RingBuffer
    {
    private:
        T mBuffer[N];
        unsigned char mHead = 0;
        unsigned char mSize = 0;

        inline unsigned char wrapIndex(unsigned char index) const
        {
            return index % N;
        }

    public:
        RingBuffer()
        {
            for (unsigned char i = 0; i < N; ++i)
                mBuffer[i] = T{};
        }

        void put(const T &value)
        {
            mBuffer[mHead] = value;
            mHead = wrapIndex(mHead + 1);
            if (mSize < N)
                mSize++;
        }

        T get(unsigned char index) const
        {
            unsigned char oldestPos = (mHead + N - mSize) % N;
            unsigned char physicalPos = wrapIndex(oldestPos + index);
            return mBuffer[physicalPos];
        }

        bool isEmpty() const
        {
            return mSize == 0;
        }

        bool isFull() const
        {
            return mSize == N;
        }

        unsigned char size() const
        {
            return mSize;
        }
    };

}
