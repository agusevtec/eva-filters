#pragma once

#include <evaRingBuffer.h>

namespace evaf
{
    /**
     * @brief Reader decorator applying a min-max (morphological) filter.
     *
     * The filter effectively removes both positive and negative impulse noise while
     * preserving edges better than a simple moving average. The output is the average
     * of the morphological opening and closing operations, providing symmetric behavior.
     *
     * This filter divides the ring buffer into N chunks of N elements each, then computes:
     * - minimax = min of chunk maximums (closing operation)
     * - maximin = max of chunk minimums (opening operation)
     * - output = (minimax + maximin) / 2
     *
     * @tparam TReader Underlying reader class (must implement getValue())
     * @tparam N Number of chunks and chunk size (total buffer size = N * N)
     *
     * @note The filter only produces filtered output after the buffer is full.
     *       Before that, values pass through unchanged.
     */
    template <class TReader, unsigned char N>
    class Minmax : public TReader
    {
        static_assert(N >= 2 && N <= 5, "N out of range 2..5");

    private:
        eva::RingBuffer<signed short, N * N> mRing;
        signed short mMaxBuffer[N];
        signed short mMinBuffer[N];

    public:
        template <typename... Args>
        Minmax(Args &&...args) : TReader(args...)
        {
            for (unsigned char i = 0; i < N; ++i)
            {
                mMaxBuffer[i] = 0;
                mMinBuffer[i] = 0;
            }
        }

        /**
         * @brief Apply input reading with min-max filtering.
         *
         * Passes values through unchanged until the ring buffer is full.
         * Once full, each new value triggers buffer update and computes:
         * output = (minimax + maximin) / 2
         *
         * @return Filtered short value
         */
        signed short getValue()
        {
            if (!TReader::isValid())
            {
                mRing.clear();
                for (unsigned char i = 0; i < N; ++i)
                {
                    mMaxBuffer[i] = 0;
                    mMinBuffer[i] = 0;
                }
                return 0;
            }

            signed short value = TReader::getValue();
            mRing.put(value);

            if (mRing.isFull())
            {
                for (unsigned char chunk = 0; chunk < N; chunk++)
                {
                    unsigned char start = chunk * N;
                    signed short maxVal = mRing.get(start);
                    signed short minVal = maxVal;

                    for (unsigned char i = 1; i < N; ++i)
                    {
                        signed short val = mRing.get(start + i);
                        if (val > maxVal)
                            maxVal = val;
                        if (val < minVal)
                            minVal = val;
                    }

                    mMaxBuffer[chunk] = maxVal;
                    mMinBuffer[chunk] = minVal;
                }

                value = (getMinimax() + getMaximin()) / 2;
            }

            return value;
        }

    private:
        signed short getMinimax() const
        {
            signed short result = mMaxBuffer[0];
            for (unsigned char i = 1; i < N; ++i)
                if (mMaxBuffer[i] < result)
                    result = mMaxBuffer[i];
            return result;
        }

        signed short getMaximin() const
        {
            signed short result = mMinBuffer[0];
            for (unsigned char i = 1; i < N; ++i)
                if (mMinBuffer[i] > result)
                    result = mMinBuffer[i];
            return result;
        }
    };

}
