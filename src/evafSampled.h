#pragma once

#include <evaHeartbeat.h>

namespace evaf
{
    /**
     * @brief Decorator that periodically polls an underlying TReader via Heartbeat and caches the result.
     *
     * No input or output clamping is applied.
     *
     * @tparam TReader Input signal reader type
     * @tparam tIntervalMs Periodic update interval in milliseconds
     */
    template <class TReader, unsigned short tIntervalMs>
    class Sampled : private eva::Heartbeat, public TReader
    {
    private:
        signed short mCachedValue = 0;

        void reset()
        {
            mCachedValue = 0;
        }

    protected:
        void onHeartbeat() override
        {
            mCachedValue = TReader::getValue();
        }

    public:
        Sampled()
            : eva::Heartbeat(tIntervalMs)
        {
            reset();
        }

        template <typename... Args>
        Sampled(unsigned short intervalMs, Args... args)
            : TReader(args...),
              eva::Heartbeat(intervalMs)
        {
            reset();
        }

        /**
         * @brief Gets the cached value retrieved on the last heartbeat tick.
         * @return Cached short value
         */
        signed short getValue()
        {
            return mCachedValue;
        }
    };

}
