#pragma once

#include <Arduino.h>

namespace evaf
{
    constexpr unsigned short kDefaultOpenCloseWindow = 5;
    constexpr unsigned short kMinOpenCloseWindow = 3;
    constexpr unsigned short kMaxOpenCloseWindow = 15;

    /**
     * @brief Reader decorator applying a morphological open-close filter.
     *
     * Computes morphological opening and closing of the sliding window and
     * returns their average:
     * - opening = dilation(erosion(x)) - removes positive impulses shorter than N
     * - closing = erosion(dilation(x)) - removes negative impulses shorter than N
     * - output  = (opening + closing) / 2
     *
     * Operates on every shift of the window, guaranteeing suppression of any
     * impulse shorter than N regardless of alignment. Sliding min and max are
     * maintained with monotonic deques, giving O(1) amortized cost per sample
     * per operation.
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
        static_assert(tWindowSize % 2 == 1,
                      "tWindowSize must be odd");

    private:
        /**
         * @brief Sliding extremum tracker over a stream of signed shorts.
         *
         * Maintains a monotonic deque of indices into a fixed-size circular
         * buffer. When the configured mode is minimum, the deque front always
         * points to the smallest value currently in the window; when maximum,
         * to the largest. Push is O(1) amortized.
         */
        struct ExtremumTracker
        {
            signed short buf[tWindowSize];
            // Deque size must be tWindowSize + 1 to distinguish between empty and full states
            unsigned short deque[tWindowSize + 1];
            unsigned short bufHead = 0;
            unsigned short dequeHead = 0;
            unsigned short dequeTail = 0;
            unsigned short count = 0;
            bool isMin = true;

            void reset(bool minMode)
            {
                bufHead = 0;
                dequeHead = 0;
                dequeTail = 0;
                count = 0;
                isMin = minMode;
                for (unsigned short i = 0; i < tWindowSize; ++i)
                    buf[i] = 0;
            }

            signed short push(signed short value)
            {
                const unsigned short pos = bufHead;
                buf[pos] = value;

                while (dequeHead != dequeTail)
                {
                    const unsigned short back = (dequeTail + (tWindowSize + 1) - 1) % (tWindowSize + 1);
                    const signed short backVal = buf[deque[back]];
                    if ((isMin && backVal >= value) || (!isMin && backVal <= value))
                        dequeTail = back;
                    else
                        break;
                }

                deque[dequeTail] = pos;
                dequeTail = (dequeTail + 1) % (tWindowSize + 1);

                bufHead = (bufHead + 1) % tWindowSize;
                if (count < tWindowSize)
                    ++count;

                if (count == tWindowSize)
                {
                    if (deque[dequeHead] == pos)
                        dequeHead = (dequeHead + 1) % (tWindowSize + 1);
                }

                return buf[deque[dequeHead]];
            }
        };

        ExtremumTracker mErosion;
        ExtremumTracker mDilation;
        ExtremumTracker mOpeningStage2;
        ExtremumTracker mClosingStage2;
        unsigned short mSampleCount = 0;

        void reset()
        {
            mErosion.reset(true);        // min
            mDilation.reset(false);      // max
            mOpeningStage2.reset(false); // max (dilation of erosion)
            mClosingStage2.reset(true);  // min (erosion of dilation)
            mSampleCount = 0;
        }

    public:
        OpenClose()
        {
            reset();
        }

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

            // Stage 1: Compute Erosion and Dilation of raw stream
            const signed short erosion = mErosion.push(raw);
            const signed short dilation = mDilation.push(raw);

            // Stage 2: Compute Opening (Dilation of Erosion) and Closing (Erosion of Dilation)
            const signed short opening = mOpeningStage2.push(erosion);
            const signed short closing = mClosingStage2.push(dilation);

            if (mSampleCount < tWindowSize)
            {
                ++mSampleCount;
                return raw;
            }

            return (opening + closing) / 2;
        }
    };

}