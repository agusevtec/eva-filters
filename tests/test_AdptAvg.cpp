#include <AUnit.h>
#include "MockReader.h"
#include <evafAdaptiveAverage.h>

test(AdaptiveSmooth_DynamicTau)
{
    evaf::AdaptiveAverage<MockReader, 1, 10> filter;

    filter.mockValue = 0;
    filter.getValue();

    filter.mockValue = 500;
    assertLess(filter.getValue(), (signed short)500);

}
