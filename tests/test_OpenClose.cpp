#include <AUnit.h>
#include "MockReader.h"
#include <evafOpenClose.h>

test(MinMax_OutlierTrimming)
{
    evaf::OpenClose<MockReader, 5> filter;

    for (int i = 0; i < 4; ++i)
    {
        filter.mockValue = 100 * (i + 1);
        filter.getValue();
    }

    assertTrue(filter.getValue() > 0);
}
