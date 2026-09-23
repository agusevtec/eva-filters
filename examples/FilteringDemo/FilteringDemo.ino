#include <evaTac.h>
#include <evaHeartbeat.h>
#include <evaStdReaders.h>

// Filter headers
#include <evafAdaptiveAverage.h>
#include <evafMedian.h>
#include <evafOpenClose.h>
#include <evafMinmax.h>
#include <evafSimpleAverage.h>
#include <evafExponentialAverage.h>
#include <evafSlewRate.h>

// ============================================================================
// FILTER SELECTION - change this line and recompile
// ============================================================================
using SelectedFilter = evaf::AdaptiveAverage<eva::ValueReader, 10, 150>;
//using SelectedFilter = evaf::Median<eva::ValueReader, 3>;
//using SelectedFilter = evaf::OpenClose<eva::ValueReader, 7>;
//using SelectedFilter = evaf::Minmax<eva::ValueReader, 2>;
//using SelectedFilter = evaf::SimpleAverage<eva::ValueReader, 3>;
//using SelectedFilter = evaf::ExponentialAverage<eva::ValueReader, 500>;
//using SelectedFilter = evaf::SlewRate<eva::ValueReader, 300>;

SelectedFilter g_filter;

class SignalDemo : public eva::Heartbeat {
private:
  enum Mode { MODE_ZERO,
              MODE_SAWTOOTH,
              MODE_SINE };

  Mode mMode = MODE_SAWTOOTH;
  unsigned long mLastModeSwitchMs = 0;
  unsigned long mStepCounter = 0;
  signed short mRawValue = 0;

  void generate() {
    unsigned long now = millis();

    // Switch mode every 4 seconds
    if (now - mLastModeSwitchMs > 4000) {
      mLastModeSwitchMs = now;
      mMode = static_cast<Mode>(random(0, 3));
    }

    mStepCounter++;

    signed short baseSignal = 0;

    switch (mMode) {
      case MODE_SAWTOOTH:
        // Sawtooth from -800 to 800
        baseSignal = -800 + (signed short)((mStepCounter * 20) % 1600);
        break;

      case MODE_SINE:
        // Sine with amplitude +-700
        baseSignal = (signed short)(700.0f * sin(mStepCounter * 0.05f));
        break;

      case MODE_ZERO:
      default:
        baseSignal = 0;
        break;
    }

    // Add a spike in 3% of cases
    if (random(0, 100) < 3) {
      baseSignal += (random(0, 2) == 0 ? 900 : -900);
    }

    mRawValue = constrain(baseSignal, -1000, 1000);
    g_filter.setValue(mRawValue);
  }

  void print() {
    // Raw shifted up by 1000, filter output down by 1000
    Serial.print("2000 0 -2000 ");
    Serial.print((int)mRawValue + 1000);
    Serial.print(' ');
    Serial.print((int)g_filter.getValue() - 1000);
    Serial.println();
  }

public:
  SignalDemo()
    : Heartbeat(100) {}

protected:
  void onHeartbeat() override {
    generate();
    print();
  }
};


void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  static SignalDemo signalDemo;
}

void loop() {
  eva::tac();
}