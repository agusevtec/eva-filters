#pragma once

#include <Arduino.h>

namespace evaf
{
    constexpr unsigned short kDefaultMinTimeConstantTicks = 1;
    constexpr unsigned short kDefaultMaxTimeConstantTicks = 15;
    constexpr unsigned short kMinTimeConstantLimit = 1;
    constexpr unsigned short kMaxTimeConstantLimit = 500;
    constexpr unsigned short kDefaultDeadzone = 3;

    /**
     * @brief Reader decorator with adaptive smoothing based on input rate of change.
     *
     * Automatically adjusts smoothing based on how fast the input is changing.
     *
     * Adaptation curve assumes an input range of -1000..1000.
     *
     * No input or output clamping is applied.
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
        static constexpr signed short kFullSpeedError = 200;
        static constexpr signed short kIdleError = 5;

        unsigned short mMinTimeConstantTicks;
        unsigned short mMaxTimeConstantTicks;
        unsigned short mDeadzone;

        signed long mTargetValue = 0;
        signed long mCurrentValue = 0;

        unsigned short calculateTimeConstant() const
        {
            signed long error = labs(mTargetValue - mCurrentValue) / 1000;

            if (error >= kFullSpeedError)
                return mMinTimeConstantTicks;
            else if (error <= kIdleError)
                return mMaxTimeConstantTicks;
            else
                return mMaxTimeConstantTicks - (unsigned short)((error - kIdleError) * (signed long)(mMaxTimeConstantTicks - mMinTimeConstantTicks) / (kFullSpeedError - kIdleError));
        }

        void reset()
        {
            mTargetValue = 0;
            mCurrentValue = 0;
        }

    public:
        AdaptiveSmooth()
            : mMinTimeConstantTicks(tMinTimeConstantTicks),
              mMaxTimeConstantTicks(tMaxTimeConstantTicks),
              mDeadzone(kDefaultDeadzone)
        {
        }

        template <typename... Args>
        AdaptiveSmooth(unsigned short aMinTimeConstantTicks,
                       unsigned short aMaxTimeConstantTicks,
                       unsigned short aDeadzone,
                       Args ...args)
            : TReader(args...),
              mMinTimeConstantTicks(constrain(aMinTimeConstantTicks, kMinTimeConstantLimit, kMaxTimeConstantLimit)),
              mMaxTimeConstantTicks(constrain(aMaxTimeConstantTicks, mMinTimeConstantTicks, kMaxTimeConstantLimit)),
              mDeadzone(aDeadzone)
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

            mTargetValue = (signed long)TReader::getValue() * 1000;

            unsigned short currentTau = calculateTimeConstant();

            if (labs(mTargetValue) <= (signed long)mDeadzone * 1000 &&
                labs(mCurrentValue) <= (signed long)mDeadzone * 1000)
            {
                if (mCurrentValue != 0)
                    mCurrentValue = 0;
            }
            else
            {
                signed long error = mTargetValue - mCurrentValue;
                mCurrentValue += error / (signed long)currentTau;
            }

            return static_cast<signed short>(mCurrentValue / 1000);
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

        void setDeadzone(unsigned short value)
        {
            mDeadzone = value;
        }

        unsigned short getDeadzone() const
        {
            return mDeadzone;
        }
    };

}
