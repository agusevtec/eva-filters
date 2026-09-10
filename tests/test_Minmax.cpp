#include <AUnit.h>
#include "MockReader.h"
#include <evafMinmax.h>

test(MinMax_OutlierTrimming)
{
    evaf::Minmax<MockReader, 2> filter;

    for (int i = 0; i < 4; ++i)
    {
        filter.mockValue = 100 * (i + 1);
        filter.getValue();
    }

    assertTrue(filter.getValue() > 0);
}
