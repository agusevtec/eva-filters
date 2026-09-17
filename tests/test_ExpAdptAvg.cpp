#include <AUnit.h>
#include "MockReader.h"
#include <evafExpAdaptiveAverage.h>

test(AdaptiveSmooth_DynamicTau)
{
    evaf::ExpAdaptiveAverage<MockReader, 1, 10> filter;

    filter.mockValue = 0;
    filter.getValue();

    filter.mockValue = 500;
    assertEqual(filter.getValue(), (signed short)500);

    filter.mockValue = 504;
    assertEqual(filter.getValue(), (signed short)500);
}
