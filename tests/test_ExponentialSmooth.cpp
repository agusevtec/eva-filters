#include <AUnit.h>
#include "MockReader.h"
#include "../src/evafExponentialSmooth.h"

test(ExponentialSmooth_StepResponse)
{
    // Pass alpha=200, then forward pin=5 to MockReader
    evaf::ExponentialSmooth<evaf_test::MockReader, 200> filter(200, 5);

    filter.mockValue = 1000;
    assertEqual(filter.getValue(), (signed short)1000);

    filter.mockValue = 0;
    // Y[k] = (200 * 0 + 800 * 1000) / 1000 = 800
    assertEqual(filter.getValue(), (signed short)800);
}
