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

    /**
     * @brief Reader decorator applying adaptive exponential smoothing.
     *
     * Unlike `ExponentialAverage` with a fixed alpha, this filter adjusts its
     * time constant on every call based on the magnitude of the error between
     * the target (raw reader value) and the current smoothed value:
     *
     * - Large error  -> small time constant -> fast response (less smoothing).
     * - Small error  -> large time constant -> slow response (more smoothing).
     *
     * The transition between the two extremes is linear between `tIdleError`
     * and `tFullSpeedError`. The time constant is expressed in call ticks and
     * does not depend on wall-clock time.
     *
     * No input or output clamping is applied. The filter assumes the reader
     * produces values in `-1000..1000` (the standard EVA signal range).
     *
     * The first valid sample initializes the filter without smoothing, so
     * there is no ramp-up from zero on startup.
     *
     * @tparam TReader Underlying reader class (must implement getValue() and isValid())
     * @tparam tMinTimeConstantTicks Minimum time constant in ticks (fast response). Default: 1
     * @tparam tMaxTimeConstantTicks Maximum time constant in ticks (heavy smoothing). Default: 15
     * @tparam tFullSpeedError Error threshold (in reader units) above which the minimum
     *                         time constant is used. Default: 1000
     * @tparam tIdleError Error threshold (in reader units) below which the maximum
     *                    time constant is used. Default: 5
     */
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

        /**
         * @brief Computes the adaptive time constant for the current error.
         *
         * The error is measured in reader units (i.e. the internal ×1000
         * scaling is divided out). The result is linearly interpolated between
         * `mMinTimeConstantTicks` and `mMaxTimeConstantTicks` over the interval
         * [`mIdleError`, `mFullSpeedError`].
         *
         * @param aTargetValue  Current target value, scaled by 1000
         * @param aCurrentValue Current smoothed value, scaled by 1000
         * @return Time constant in ticks, in the range [min, max]
         */
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

        /**
         * @brief Clears the filter state so the next valid sample re-initializes it.
         */
        void reset()
        {
            mCurrentValue = 0;
            mInitialized = false;
        }

    public:
        /**
         * @brief Default constructor. Uses the template parameters as configuration.
         */
        AdaptiveAverage()
            : mMinTimeConstantTicks(tMinTimeConstantTicks),
              mMaxTimeConstantTicks(tMaxTimeConstantTicks),
              mFullSpeedError(tFullSpeedError),
              mIdleError(tIdleError)
        {
        }

        /**
         * @brief Runtime-configurable constructor.
         *
         * Allows overriding the time constants and error thresholds at runtime.
         * Values are clamped to their respective limits. Additional arguments
         * are forwarded to the underlying TReader constructor.
         *
         * @param aMinTimeConstantTicks Minimum time constant in ticks (clamped)
         * @param aMaxTimeConstantTicks Maximum time constant in ticks (clamped, >= min)
         * @param aFullSpeedError Error threshold for fast response (clamped)
         * @param aIdleError Error threshold for heavy smoothing (clamped, <= full-speed)
         * @param args Additional arguments forwarded to TReader
         */
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

        /**
         * @brief Gets the adaptively smoothed value.
         *
         * If the underlying reader is invalid, the filter resets its state and
         * returns 0. The first valid sample initializes the internal value
         * without smoothing. Subsequent samples are smoothed with a time
         * constant chosen by the current error magnitude.
         *
         * @return Smoothed value in the reader's range
         */
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

        /**
         * @brief Sets the minimum time constant (fast-response regime).
         * @param value Time constant in ticks, clamped to [kMinTimeConstantLimit, kMaxTimeConstantLimit]
         */
        void setMinTimeConstantTicks(unsigned short value)
        {
            mMinTimeConstantTicks = constrain(value, kMinTimeConstantLimit, kMaxTimeConstantLimit);
        }

        /**
         * @brief Gets the minimum time constant.
         * @return Time constant in ticks
         */
        unsigned short getMinTimeConstantTicks() const
        {
            return mMinTimeConstantTicks;
        }

        /**
         * @brief Sets the maximum time constant (heavy-smoothing regime).
         * @param value Time constant in ticks, clamped to [mMinTimeConstantTicks, kMaxTimeConstantLimit]
         */
        void setMaxTimeConstantTicks(unsigned short value)
        {
            mMaxTimeConstantTicks = constrain(value, mMinTimeConstantTicks, kMaxTimeConstantLimit);
        }

        /**
         * @brief Gets the maximum time constant.
         * @return Time constant in ticks
         */
        unsigned short getMaxTimeConstantTicks() const
        {
            return mMaxTimeConstantTicks;
        }

        /**
         * @brief Sets the error threshold above which the minimum time constant is used.
         * @param value Error in reader units, clamped to [mIdleError, kMaxErrorLimit]
         */
        void setFullSpeedError(unsigned short value)
        {
            mFullSpeedError = constrain(value, mIdleError, kMaxErrorLimit);
        }

        /**
         * @brief Gets the full-speed error threshold.
         * @return Error in reader units
         */
        unsigned short getFullSpeedError() const
        {
            return mFullSpeedError;
        }

        /**
         * @brief Sets the error threshold below which the maximum time constant is used.
         * @param value Error in reader units, clamped to [kMinErrorLimit, mFullSpeedError]
         */
        void setIdleError(unsigned short value)
        {
            mIdleError = constrain(value, kMinErrorLimit, mFullSpeedError);
        }

        /**
         * @brief Gets the idle error threshold.
         * @return Error in reader units
         */
        unsigned short getIdleError() const
        {
            return mIdleError;
        }
    };

}