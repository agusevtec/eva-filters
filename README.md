# EVA Filters (eva-filters)

C++ Signal filtering library designed for the **EVA** ecosystem in Arduino.

`EVA Filters` provides compile-time chainable filter decorators to clean and smooth analog signals. 

---

## Key Concepts

- **Discrete Time (Call Ticks)**: Filters do not read system time. All time-dependent parameters (like time constants or slew limits) are configured in units of call ticks, making execution deterministic, lightweight, and independent of hardware timers.
- **Decoupled Timing via `Sampled`**: Time-dependent caching is cleanly separated from filter logic. Wrap any chain in `evaf::Sampled` to enforce periodic updates. Between ticks, `Sampled` returns the cached value without touching the underlying reader.


---

## Available Filters

All classes reside in the `evaf` namespace and follow the decorator pattern: each filter wraps a `TReader` that implements `signed short getValue()` and `bool isValid()`.

| Filter Class | Description | Template Parameters |
| :--- | :--- | :--- |
| `evaf::Median` | Spike elimination over a sliding window | `<class TReader, unsigned char tWindowSize = 5>` |
| `evaf::SimpleAverage` | Simple Moving Average (SMA) over a sliding window | `<class TReader, unsigned char tWindowSize = 8>` |
| `evaf::ExponentialAverage` | Exponential Moving Average (EMA) with fixed alpha | `<class TReader, unsigned short tAlpha = 200>` |
| `evaf::AdaptiveAverage` | Dynamic EMA that adjusts smoothing by error magnitude | `<class TReader, unsigned short tMinTimeConstantTicks = 1, unsigned short tMaxTimeConstantTicks = 15, unsigned short tFullSpeedError = 200, unsigned short tIdleError = 5>` |
| `evaf::SlewRate` | Rate limiter restricting maximum value delta per tick | `<class TReader, signed short tMaxStepPerTick = 30>` |
| `evaf::Minmax` | Trims min and max outliers before averaging | `<class TReader, unsigned char tWindowSize = 5>` |
| `evaf::OpenClose` | Morphological open-close filter (naive reference implementation) | `<class TReader, unsigned short tWindowSize = 5>` |
| `evaf::Sampled` | Caches filter outputs at a fixed time interval | `<class TReader, unsigned short tIntervalMs>` |

---

## Parameter Notes

- `tAlpha` (1..1000): EMA smoothing factor. `1000` = no filtering, `100` = heavy smoothing. Relationship to time constant: `alpha ≈ 1000 / tau`.
- `tMinTimeConstantTicks` / `tMaxTimeConstantTicks`: time constant (in ticks) used by `AdaptiveAverage` when the error is large / small respectively.
- `tFullSpeedError` / `tIdleError`: error thresholds (in reader units) that define the interpolation range for the adaptive time constant. Both are clamped at runtime to `1..30000`, and `tIdleError ≤ tFullSpeedError`.
- `tMaxStepPerTick`: maximum absolute change of the output per call, in reader units.
- `tWindowSize`: number of samples in the sliding window. Must be odd for `OpenClose`.
- `tIntervalMs`: heartbeat period in milliseconds. Requires `eva::tac()` to be called regularly (see example).

---

## Quick Start


### Example: Cleaned Joystick Input

```cpp
#include <evaTac.h>
#include <evaJoystick.h>
#include <evaHeartbeat.h>

#include <evafFilters.h>
#include <evafSampled.h>

// Raw ADC Pin -> Median Filter -> Exponential Smooth -> 10ms Heartbeat Cache
using CleanedPinReader = evaf::Sampled<
  evaf::ExponentialAverage<
    evaf::Median<
      eva::AnalogPinReader<A0, INPUT>,
      5  // Window size = 5
      >,
    200  // Alpha = 200 (moderate smoothing)
    >,
  10  // Heartbeat update interval = 10ms
  >;

class App : public eva::Heartbeat {
  eva::Joystick<CleanedPinReader> joystick;
public:
  App()
    : eva::Heartbeat(100) {}
  void onHeartbeat() {
    signed short output = joystick.getValue();
    //...
  }
};

void setup()
{
   static App app;
}

void loop()
{
  eva::tac();
}
```

---

 All storage is statically sized through template parameters, or through runtime parameters of the same name.

## Installation

Using Arduino Library Manager

Open Arduino IDE

Go to Sketch -> Include Library -> Manage Libraries

Search for "eva-filters"

Click Install

## License

MIT License
