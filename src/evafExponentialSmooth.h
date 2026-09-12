#pragma once

#include <Arduino.h>

namespace evaf
{
    /**
     * @brief Reader decorator applying Exponential Moving Average (EMA) filtering.
     *
     * Formula: Y[k] = alpha * X[k] + (1 - alpha) * Y[k-1]
     * Uses fixed-point arithmetic (scaled by 1000) for fast execution on MCUs.
     *
     * No input or output clamping is applied.
     *
     * @tparam TReader Underlying reader class (must implement getValue())
     * @tparam tAlpha Smoothing factor from 1 to 1000 (1000 = no filtering, 100 = heavy smoothing)
     */
    template <class TReader, unsigned short tAlpha = 200>
    class ExponentialSmooth : public TReader
    {
        static_assert(tAlpha >= 1 && tAlpha <= 1000, "tAlpha must be between 1 and 1000");

    private:
        unsigned short mAlpha;
        signed long mCurrentValue = 0;
        bool mInitialized = false;

        void reset()
        {
            mCurrentValue = 0;
            mInitialized = false;
        }

    public:
        ExponentialSmooth()
            : mAlpha(tAlpha) {}

        template <typename... Args>
        ExponentialSmooth(unsigned short aAlpha, Args ...args)
            : TReader(args...), mAlpha(constrain(aAlpha, 1, 1000)) {}

        /**
         * @brief Gets exponential smoothed reader value
         * @return Smoothed value
         */
        signed short getValue()
        {
            if (!TReader::isValid())
            {
                reset();
                return 0;
            }

            signed short raw = TReader::getValue();

            if (!mInitialized)
            {
                mCurrentValue = (signed long)raw * 1000;
                mInitialized = true;
                return raw;
            }

            signed long diff = (signed long)raw * 1000 - mCurrentValue;
            mCurrentValue += diff * (signed long)mAlpha / 1000;

            return static_cast<signed short>(mCurrentValue / 1000);
        }

        /**
         * @brief Sets dynamic smoothing alpha factor
         * @param alpha Value from 1 to 1000
         */
        void setAlpha(unsigned short alpha)
        {
            mAlpha = constrain(alpha, 1, 1000);
        }

        /**
         * @brief Gets current alpha factor
         * @return Value from 1 to 1000
         */
        unsigned short getAlpha() const
        {
            return mAlpha;
        }
    };

}
