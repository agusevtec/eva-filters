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
        signed short mBuffer[tWindowSize];
        unsigned short mIndex = 0;
        bool mBufferFull = false;
        signed short mFilteredValue = 0;

        signed short calculateMedian()
        {
            signed short sorted[tWindowSize];
            unsigned short count = mBufferFull ? tWindowSize : mIndex + 1;

            for (unsigned short i = 0; i < count; i++)
                sorted[i] = mBuffer[i];

            for (unsigned short i = 0; i < count - 1; i++)
            {
                for (unsigned short j = 0; j < count - i - 1; j++)
                {
                    if (sorted[j] > sorted[j + 1])
                    {
                        signed short temp = sorted[j];
                        sorted[j] = sorted[j + 1];
                        sorted[j + 1] = temp;
                    }
                }
            }

            return sorted[count / 2];
        }

    public:
        template <typename... Args>
        Median(Args &&...args) : TReader(args...)
        {
            for (unsigned short i = 0; i < tWindowSize; i++)
                mBuffer[i] = 0;
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

            mBuffer[mIndex] = constrain(TReader::getValue(), -1000, 1000);
            mIndex++;

            if (mIndex >= tWindowSize)
            {
                mIndex = 0;
                mBufferFull = true;
            }

            if (mBufferFull || mIndex > tWindowSize / 2)
            {
                mFilteredValue = calculateMedian();
            }

            return mFilteredValue;
        }

        /**
         * @brief Reset the filter buffer.
         */
        void reset()
        {
            mIndex = 0;
            mBufferFull = false;
            mFilteredValue = 0;
            for (unsigned short i = 0; i < tWindowSize; i++)
                mBuffer[i] = 0;
        }
    };

}
