#include <evaTac.h>
#include <evaJoystick.h>
#include <evafExponentialSmooth.h>
#include <evafMedian.h>
#include <evafHeartbeatDecor.h>

// Chain: Raw Analog ADC -> Median Filter (5 samples) -> Exponential Smooth (10 ticks) -> Heartbeat (10ms)
using CleanedPinReader = evaf::HeartbeatDecor < evaf::ExponentialSmooth<evaf::Median<eva::AnalogPinReader<A0, INPUT>, 5>, 10>, 10;

// Application Joystick driven by cleaned signal
eva::Joystick joystick;

void setup()
{
    Serial.begin(9600);
}

void loop()
{
    eva::tac();

    // Read smoothed signal in standard EVA range (-1000..1000)
    signed short val = joystick.getValue();
    (void)val;
}
