#pragma once

#include <evaRingBuffer.h>

namespace evaf
{
    /**
     * @brief Reader decorator applying a moving average (sliding window) filter.
     *
     * No input or output clamping is applied.
     *
     * @tparam TReader Underlying reader class (must implement getValue())
     * @tparam N Window size (number of values to average). Must be >= 1.
     *
     * @note The filter only produces filtered output after the buffer is full.
     *       Before that, values pass through unchanged.
     */
    template <class TReader, unsigned short N>
    class SimpleAverage : public TReader
    {
        static_assert(N >= 1 && N <= 32, "N out of range 1..32");

    private:
        eva::RingBuffer<signed short, N> mRing;
        signed long mSum = 0;

        void reset()
        {
            mRing.clear();
            mSum = 0;
        }

    public:
        template <typename... Args>
        SimpleAverage(Args... args) : TReader(args...)
        {
        }

        /**
         * @brief Gets moving average filtered value.
         * @return Filtered short value
         */
        signed short getValue()
        {
            if (!TReader::isValid())
            {
                reset();
                return 0;
            }

            signed short value = TReader::getValue();

            if (mRing.isFull())
                mSum -= mRing.get(0);

            mRing.put(value);
            mSum += value;

            if (mRing.isFull())
                value = static_cast<signed short>(mSum / N);

            return value;
        }
    };

}
