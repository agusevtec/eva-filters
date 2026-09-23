#pragma once

#include <Arduino.h>

namespace evaf
{
    constexpr unsigned short kDefaultMinTimeConstantTicks = 1;
    constexpr unsigned short kDefaultMaxTimeConstantTicks = 15;
    constexpr unsigned short kMinTimeConstantLimit = 1;
    constexpr unsigned short kMaxTimeConstantLimit = 500;

    constexpr unsigned short kDefaultFullSpeedError = 1000;
    constexpr unsigned short kDefaultIdleError = 5;
    constexpr unsigned short kMinErrorLimit = 1;
    constexpr unsigned short kMaxErrorLimit = 30000;

    template <class TReader,
              unsigned short tMinTimeConstantTicks = kDefaultMinTimeConstantTicks,
              unsigned short tMaxTimeConstantTicks = kDefaultMaxTimeConstantTicks,
              unsigned short tFullSpeedError = kDefaultFullSpeedError,
              unsigned short tIdleError = kDefaultIdleError>
    class AdaptiveAverage : public TReader
    {
        static_assert(tMinTimeConstantTicks >= kMinTimeConstantLimit && tMinTimeConstantTicks <= kMaxTimeConstantLimit,
                      "tMinTimeConstantTicks out of range");
        static_assert(tMaxTimeConstantTicks >= tMinTimeConstantTicks && tMaxTimeConstantTicks <= kMaxTimeConstantLimit,
                      "tMaxTimeConstantTicks must be >= tMinTimeConstantTicks");
        static_assert(tIdleError >= kMinErrorLimit && tIdleError <= kMaxErrorLimit,
                      "tIdleError out of range");
        static_assert(tFullSpeedError >= tIdleError && tFullSpeedError <= kMaxErrorLimit,
                      "tFullSpeedError must be >= tIdleError");

    private:
        unsigned short mMinTimeConstantTicks;
        unsigned short mMaxTimeConstantTicks;
        unsigned short mFullSpeedError;
        unsigned short mIdleError;

        signed long mCurrentValue = 0;
        bool mInitialized = false;

        unsigned short calculateTimeConstant(signed long aTargetValue, signed long aCurrentValue) const
        {
            signed long error = labs(aTargetValue - aCurrentValue) / 1000;

            if (error >= (signed long)mFullSpeedError)
                return mMinTimeConstantTicks;
            else if (error <= (signed long)mIdleError)
                return mMaxTimeConstantTicks;
            else
                return mMaxTimeConstantTicks - (unsigned short)((error - (signed long)mIdleError) * (signed long)(mMaxTimeConstantTicks - mMinTimeConstantTicks) / (signed long)(mFullSpeedError - mIdleError));
        }

        void reset()
        {
            mCurrentValue = 0;
            mInitialized = false;
        }

    public:
        AdaptiveAverage()
            : mMinTimeConstantTicks(tMinTimeConstantTicks),
              mMaxTimeConstantTicks(tMaxTimeConstantTicks),
              mFullSpeedError(tFullSpeedError),
              mIdleError(tIdleError)
        {
        }

        template <typename... Args>
        AdaptiveAverage(unsigned short aMinTimeConstantTicks,
                       unsigned short aMaxTimeConstantTicks,
                       unsigned short aFullSpeedError,
                       unsigned short aIdleError,
                       Args ...args)
            : TReader(args...),
              mMinTimeConstantTicks(constrain(aMinTimeConstantTicks, kMinTimeConstantLimit, kMaxTimeConstantLimit)),
              mMaxTimeConstantTicks(constrain(aMaxTimeConstantTicks, mMinTimeConstantTicks, kMaxTimeConstantLimit)),
              mFullSpeedError(constrain(aFullSpeedError, kMinErrorLimit, kMaxErrorLimit)),
              mIdleError(constrain(aIdleError, kMinErrorLimit, mFullSpeedError))
        {
        }

        signed short getValue()
        {
            if (!TReader::isValid())
            {
                reset();
                return 0;
            }

            signed long target = (signed long)TReader::getValue() * 1000;

            if (!mInitialized)
            {
                mCurrentValue = target;
                mInitialized = true;
                return static_cast<signed short>(target / 1000);
            }

            unsigned short currentTau = calculateTimeConstant(target, mCurrentValue);

            signed long error = target - mCurrentValue;
            signed long step = error / (signed long)currentTau;

            if (step == 0)
                mCurrentValue = target;
            else
                mCurrentValue += step;

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

        void setFullSpeedError(unsigned short value)
        {
            mFullSpeedError = constrain(value, mIdleError, kMaxErrorLimit);
        }

        unsigned short getFullSpeedError() const
        {
            return mFullSpeedError;
        }

        void setIdleError(unsigned short value)
        {
            mIdleError = constrain(value, kMinErrorLimit, mFullSpeedError);
        }

        unsigned short getIdleError() const
        {
            return mIdleError;
        }
    };

}