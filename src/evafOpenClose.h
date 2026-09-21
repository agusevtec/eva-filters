#pragma once

#include <evaRingBuffer.h>

namespace evaf
{
    constexpr unsigned short kDefaultOpenCloseWindow = 5;
    constexpr unsigned short kMinOpenCloseWindow = 3;
    constexpr unsigned short kMaxOpenCloseWindow = 15;

    /**
     * @brief Reader decorator applying a morphological open-close filter.
     *
     * Naive reference implementation used as a baseline for testing.
     * Computes morphological opening and closing of the sliding window and
     * returns their average:
     * - opening = dilation(erosion(x))
     * - closing = erosion(dilation(x))
     * - output  = (opening + closing) / 2
     *
     * No input or output clamping is applied.
     *
     * @tparam TReader Underlying reader class (must implement getValue())
     * @tparam tWindowSize Filter window size (odd number). Default: 5
     */
    template <class TReader, unsigned short tWindowSize = kDefaultOpenCloseWindow>
    class OpenClose : public TReader
    {
        static_assert(tWindowSize >= kMinOpenCloseWindow && tWindowSize <= kMaxOpenCloseWindow,
                      "tWindowSize out of range");

    private:
        eva::RingBuffer<signed short, tWindowSize> mRaw;
        eva::RingBuffer<signed short, tWindowSize> mErosion;
        eva::RingBuffer<signed short, tWindowSize> mDilation;

        void reset()
        {
            mRaw.clear();
            mErosion.clear();
            mDilation.clear();
        }

        signed short minOf(const eva::RingBuffer<signed short, tWindowSize>& buf) const
        {
            signed short result = buf.get(0);
            for (unsigned short i = 1; i < buf.size(); ++i)
            {
                signed short v = buf.get(i);
                if (v < result)
                    result = v;
            }
            return result;
        }

        signed short maxOf(const eva::RingBuffer<signed short, tWindowSize>& buf) const
        {
            signed short result = buf.get(0);
            for (unsigned short i = 1; i < buf.size(); ++i)
            {
                signed short v = buf.get(i);
                if (v > result)
                    result = v;
            }
            return result;
        }

    public:
        template <typename... Args>
        OpenClose(Args &&...args) : TReader(args...)
        {
            reset();
        }

        /**
         * @brief Apply input reading with morphological open-close filtering.
         *
         * Before the window is full, the raw value is passed through unchanged.
         * Once full, output = (opening + closing) / 2.
         *
         * @return Filtered short value
         */
        signed short getValue()
        {
            if (!TReader::isValid())
            {
                reset();
                return 0;
            }

            const signed short raw = TReader::getValue();

            mRaw.put(raw);

            const signed short erosion  = minOf(mRaw);
            const signed short dilation = maxOf(mRaw);

            mErosion.put(erosion);
            mDilation.put(dilation);

            if (!mRaw.isFull())
                return raw;

            const signed short opening = maxOf(mErosion);
            const signed short closing = minOf(mDilation);

            return static_cast<signed short>(
                ((signed long)opening + (signed long)closing) / 2);
        }
    };

}