#pragma once

#include <Arduino.h>

namespace evaf
{
    constexpr unsigned short kDefaultMinTimeConstantTicks = 1;
    constexpr unsigned short kDefaultMaxTimeConstantTicks = 15;
    constexpr unsigned short kMinTimeConstantLimit = 1;
    constexpr unsigned short kMaxTimeConstantLimit = 500;
    constexpr unsigned short kDeadzone = 3;

    /**
     * @brief Reader decorator with adaptive smoothing based on input rate of change.
     *
     * Automatically adjusts smoothing based on how fast the input is changing.
     *
     * @tparam TReader Underlying reader class (must implement getValue())
     * @tparam tMinTimeConstantTicks Minimum time constant in ticks (fast response). Default: 1
     * @tparam tMaxTimeConstantTicks Maximum time constant in ticks (heavy smoothing). Default: 15
     */
    template <class TReader,
              unsigned short tMinTimeConstantTicks = kDefaultMinTimeConstantTicks,
              unsigned short tMaxTimeConstantTicks = kDefaultMaxTimeConstantTicks>
    class AdaptiveSmooth : public TReader
    {
        static_assert(tMinTimeConstantTicks >= kMinTimeConstantLimit && tMinTimeConstantTicks <= kMaxTimeConstantLimit,
                      "tMinTimeConstantTicks out of range");
        static_assert(tMaxTimeConstantTicks >= tMinTimeConstantTicks && tMaxTimeConstantTicks <= kMaxTimeConstantLimit,
                      "tMaxTimeConstantTicks must be >= tMinTimeConstantTicks");

    private:
        unsigned short mMinTimeConstantTicks;
        unsigned short mMaxTimeConstantTicks;

        signed short mTargetValue = 0;
        signed short mCurrentValue = 0;

        unsigned short calculateTimeConstant()
        {
            signed short error = abs(mTargetValue - mCurrentValue);

            if (error >= 200)
                return mMinTimeConstantTicks;
            else if (error <= 5)
                return mMaxTimeConstantTicks;
            else
            {
                return mMaxTimeConstantTicks - ((error - 5) * (mMaxTimeConstantTicks - mMinTimeConstantTicks) / 195);
            }
        }

    public:
        template <typename... Args>
        AdaptiveSmooth(unsigned short aMinTimeConstantTicks = tMinTimeConstantTicks, unsigned short aMaxTimeConstantTicks = tMaxTimeConstantTicks, Args &&...args)
            : TReader(args...),
              mMinTimeConstantTicks(constrain(aMinTimeConstantTicks, kMinTimeConstantLimit, kMaxTimeConstantLimit)),
              mMaxTimeConstantTicks(constrain(aMaxTimeConstantTicks, mMinTimeConstantTicks, kMaxTimeConstantLimit))
        {
        }

        /**
         * @brief Gets adaptively smoothed value
         * @return Smoothed value
         */
        signed short getValue()
        {
            if (!TReader::isValid())
            {
                reset();
                return 0;
            }

            mTargetValue = constrain(TReader::getValue(), -1000, 1000);
            unsigned short currentTau = calculateTimeConstant();

            if (abs(mTargetValue) <= kDeadzone && abs(mCurrentValue) <= kDeadzone)
            {
                if (mCurrentValue != 0)
                    mCurrentValue = 0;
            }
            else
            {
                signed long step = (signed long)(mTargetValue - mCurrentValue) * 1000 / currentTau;
                mCurrentValue += step / 1000;
                mCurrentValue = constrain(mCurrentValue, -1000, 1000);
            }

            return mCurrentValue;
        }

        void reset(signed short initialValue = 0)
        {
            mTargetValue = initialValue;
            mCurrentValue = initialValue;
        }

        void setMinTimeConstantTicks(unsigned short value)
        {
            mMinTimeConstantTicks = constrain(value, kMinTimeConstantLimit, kMaxTimeConstantLimit);
        }

        unsigned short getMinTimeConstantTicks() const
        {
            return mMinTimeConstantTicks;
        }

        void setMaxTimeConstantTicks(unsigned short value)
        {
            mMaxTimeConstantTicks = constrain(value, mMinTimeConstantTicks, kMaxTimeConstantLimit);
        }

        unsigned short getMaxTimeConstantTicks() const
        {
            return mMaxTimeConstantTicks;
        }
    };

}
