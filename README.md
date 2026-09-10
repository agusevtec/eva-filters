# EVA Filters (evaf)

A lightweight, header-only C++ digital signal processing (DSP) library designed for the **Extremely Versatile Architecture (EVA)** ecosystem.

`EVA Filters` provides compile-time chainable filter decorators to clean, smooth, and constrain analog and digital signals. Built specifically for microcontrollers, it features **zero dynamic memory allocation** and operates entirely without `millis()` or hardware timer dependencies.

---

## Key Concepts

- **Standardized Range (`-1000..1000`)**: All filters are optimized around the standard EVA signal scale. Bipolar and unipolar signals map seamlessly to this range.
- **Discrete Time (Call Ticks)**: Filters do not read system time. All time-dependent parameters (like time constants or slew limits) are configured in units of call ticks, making execution deterministic, lightweight, and independent of hardware timers.
- **On-Demand Processing**: Filters process data inside `getValue()`. Unused filters consume zero CPU cycles.
- **Decoupled Timing via `HeartbeatDecor`**: Time-dependent caching is cleanly separated from filter logic. Wrap any chain in `evaf::HeartbeatDecor` to enforce periodic updates.

---

## Available Filters

All classes reside in the `evaf` namespace.

| Filter Class | Description | Template Parameters |
| :--- | :--- | :--- |
| `evaf::Median` | Spike elimination over a sliding window | `<class TReader, unsigned char tWindowSize = 5>` |
| `evaf::ExponentialSmooth` | Exponential Moving Average (EMA) filter | `<class TReader, unsigned short tTimeConstantTicks = 10>` |
| `evaf::AdaptiveSmooth` | Dynamic EMA adjusting filter strength by error magnitude | `<class TReader, unsigned short tMinTimeConstantTicks = 1, unsigned short tMaxTimeConstantTicks = 15>` |
| `evaf::SlewRate` | Rate limiter restricting maximum value delta per tick | `<class TReader, signed short tMaxStepPerTick = 30>` |
| `evaf::SlidingWindow` | Simple Moving Average (SMA) filter | `<class TReader, unsigned char tWindowSize = 8>` |
| `evaf::Minmax` | Trims min and max outlier values before averaging | `<class TReader, unsigned char tWindowSize = 5>` |
| `evaf::HeartbeatDecor` | Caches filter outputs at a fixed time interval | `<class TReader, unsigned short tIntervalMs>` |

---

## Quick Start

### Installation
1. Ensure `eva-core-sk` is installed in your Arduino libraries folder.
2. Copy `EVA Filters` into your `libraries` directory.
3. Include `<EVAFilters.h>` in your project.

### Example: Cleaned Joystick Input

```cpp
#include <evaTac.h>
#include <evaJoystick.h>
#include <EVAFilters.h>

// Build a zero-cost processing pipeline:
// Raw ADC Pin -> Median Filter -> Exponential Smooth -> 10ms Heartbeat Cache
using CleanedPinReader = evaf::HeartbeatDecor<
    evaf::ExponentialSmooth<
        evaf::Median<
            eva::AnalogPinReader<A0, INPUT>,
            5 // Window size = 5
        >,
        10 // Time constant = 10 ticks
    >,
    10 // Heartbeat update interval = 10ms
>;

eva::Joystick<CleanedPinReader> joystick;

void setup() {
    Serial.begin(9600);
}

void loop() {
    eva::tac(); // Drive update chain
    
    signed short output = joystick.getValue(); // Cleaned signal in -1000..1000 range
}
```

---

## Ecosystem

`EVA Filters` is part of the **Extremely Versatile Architecture** ecosystem:
- **EVA Core / Survival Kit**: [eva-core-sk](https://github.com/agusevtec/eva-core-sk)
- **EVA Motors**: [eva-motors](https://github.com/agusevtec/eva-motors)
- **EVA Boxy**: [eva-boxy](https://github.com/agusevtec/eva-boxy)

---

## License

MIT License