#include <evaTac.h>
#include <evaJoystick.h>
#include <evafAdaptiveAverage.h>
#include <evafMedian.h>
#include <evafSampled.h>

// Chain: Raw Analog ADC -> Median Filter (5 samples) -> Exponential Smooth (10 ticks) -> Heartbeat (10ms)
using CleanedPinReader = evaf::Sampled< evaf::AdaptiveAverage<evaf::Median<eva::PinJoystick<A0, INPUT, 0, 512, 1024>>>, 10>;

// ============================================================================
// App prints the filtered joystick value every 100 ms.
//
// NOTE: the filter chain runs at a much higher rate than the printer.
// Sampled<..., 10> is a Heartbeat with a 10 ms period, so every sample of
// the ADC goes through Median and AdaptiveAverage every 10 ms, while
// this App only reads the latest cached value every 100 ms. The 100 ms
// print interval is purely a display decision; it does not slow down the
// filtering pipeline. Do not confuse the two rates:
//   - 10 ms  -> actual sampling / filtering rate (driven by Sampled)
//   - 100 ms -> serial output rate (driven by App)
// ============================================================================
class App : public eva::Heartbeat {
  CleanedPinReader joystick;
public:
  App()
    : Heartbeat(100) {}

  void onHeartbeat() override {
    // Reads the latest value produced by the 10 ms filtering chain.
    // No filtering happens here - just display.
    Serial.println(joystick.getValue());
  }
};

void setup() {
  Serial.begin(9600);
}

void loop() {
  eva::tac();
}