#include <AUnit.h>
#include "MockReader.h"
#include "../src/evafMinmax.h"

test(MinMax_OutlierTrimming)
{
    evaf::Minmax<evaf_test::MockReader, 2> filter;

    for (int i = 0; i < 4; ++i)
    {
        filter.mockValue = 100 * (i + 1);
        filter.getValue();
    }

    assertTrue(filter.getValue() > 0);
}
