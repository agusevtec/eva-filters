#pragma once

#include <Arduino.h>

namespace evaf
{
    /**
     * @brief Reader decorator that limits maximum rate of change (slew rate / ramp).
     *
     * Prevents sharp steps by capping maximum delta per call tick.
     *
     * @tparam TReader Underlying reader class (must implement getValue())
     * @tparam tMaxStepPerTick Maximum allowed change per call tick. Default: 50
     */
    template <class TReader, unsigned short tMaxStepPerTick = 50>
    class SlewRate : public TReader
    {
        static_assert(tMaxStepPerTick >= 1, "tMaxStepPerTick must be >= 1");

    private:
        unsigned short mMaxStep;
        signed short mCurrentValue = 0;
        bool mInitialized = false;

        void reset()
        {
            mCurrentValue = 0;
            mInitialized = false;
        }

    public:
        SlewRate()
            : mMaxStep(tMaxStepPerTick) {}

        template <typename... Args>
        SlewRate(unsigned short aMaxStepPerTick, Args... args)
            : TReader(args...), mMaxStep(aMaxStepPerTick) {}

        /**
         * @brief Gets slew-rate limited value
         * @return Value constrained by rate limit
         */
        signed short getValue()
        {
            if (!TReader::isValid())
            {
                reset();
                return 0;
            }

            signed short target = TReader::getValue();

            if (!mInitialized)
            {
                mCurrentValue = target;
                mInitialized = true;
                return target;
            }

            signed long delta = (signed long)target - (signed long)mCurrentValue;

            if (delta > (signed long)mMaxStep)
                mCurrentValue += (signed short)mMaxStep;
            else if (delta < -(signed long)mMaxStep)
                mCurrentValue -= (signed short)mMaxStep;
            else
                mCurrentValue = target;

            return mCurrentValue;
        }

        /**
         * @brief Sets maximum step per call tick
         * @param maxStep Step size limit
         */
        void setMaxStep(unsigned short maxStep)
        {
            mMaxStep = maxStep;
        }

        /**
         * @brief Gets current step size limit
         * @return Step limit per tick
         */
        unsigned short getMaxStep() const
        {
            return mMaxStep;
        }
    };

}
