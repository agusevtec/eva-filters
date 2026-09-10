#pragma once

#include <evaHeartbeat.h>

namespace evaf
{
    /**
     * @brief Decorator that periodically polls an underlying TReader via Heartbeat and caches the result.
     *
     * @tparam TReader Input signal reader type
     * @tparam tIntervalMs Periodic update interval in milliseconds
     */
    template <class TReader, unsigned short tIntervalMs>
    class HeartbeatDecor : public virtual eva::Heartbeat, public TReader
    {
    private:
        signed short mCachedValue = 0;

    protected:
        void onHeartbeat() override
        {
            mCachedValue = TReader::getValue();
        }

    public:
        template <typename... Args>
        HeartbeatDecor(unsigned short intervalMs = tIntervalMs, Args &&...args)
            : TReader(args...),
              eva::Heartbeat(intervalMs)
        {
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
