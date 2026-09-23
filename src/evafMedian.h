#pragma once

#include <Arduino.h>

namespace evaf
{
    constexpr unsigned short kDefaultWindowSize = 5;
    constexpr unsigned short kMinWindowSize = 3;
    constexpr unsigned short kMaxWindowSize = 15;

    /**
     * @brief Reader decorator applying median filtering.
     *
     * Samples the input from TReader and applies median filtering
     * to remove spikes and impulse noise.
     *
     * @tparam TReader Underlying reader class (must implement getValue())
     * @tparam tWindowSize Filter window size (odd number). Default: 5
     */
    template <class TReader, unsigned short tWindowSize = kDefaultWindowSize>
    class Median : public TReader
    {
        static_assert(tWindowSize >= kMinWindowSize && tWindowSize <= kMaxWindowSize,
                      "tWindowSize out of range");
        static_assert(tWindowSize % 2 == 1,
                      "tWindowSize must be odd");

    private:
        signed short mSorted[tWindowSize];
        unsigned short mCount = 0;
        unsigned short mHead = 0;

        void insertSorted(signed short value)
        {
            unsigned short pos = mCount;
            while (pos > 0 && mSorted[pos - 1] > value)
            {
                mSorted[pos] = mSorted[pos - 1];
                --pos;
            }
            mSorted[pos] = value;
            ++mCount;
        }

        void removeSorted(signed short value)
        {
            unsigned short pos = 0;
            while (pos < mCount && mSorted[pos] != value)
                ++pos;

            if (pos >= mCount)
                return;

            for (unsigned short i = pos; i + 1 < mCount; ++i)
                mSorted[i] = mSorted[i + 1];

            --mCount;
        }

        void reset()
        {
            mCount = 0;
            mHead = 0;
            for (unsigned short i = 0; i < tWindowSize; ++i)
                mSorted[i] = 0;
        }

    public:

        template <typename... Args>
        Median(Args ...args): TReader(args...)
        {
            reset();
        }

        /**
         * @brief Gets median filtered value.
         * @return Filtered short value
         */
        signed short getValue()
        {
            if (!TReader::isValid())
            {
                reset();
                return 0;
            }

            signed short incoming = TReader::getValue();

            if (mCount < tWindowSize)
            {
                insertSorted(incoming);
            }
            else
            {
                signed short outgoing = mSorted[mHead];
                removeSorted(outgoing);
                insertSorted(incoming);
                mHead = (mHead + 1) % tWindowSize;
            }

            return mSorted[mCount / 2];
        }
    };

}
