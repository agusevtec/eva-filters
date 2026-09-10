#include <AUnit.h>
#include "MockReader.h"
#include <evafMedian.h>

test(Median_SpikeSuppression)
{
    evaf::Median<MockReader, 5> filter(13); // Forward pin 13 to MockReader

    filter.mockValue = 100;
    for (int i = 0; i < 4; ++i)
    {
        filter.getValue();
    }
    assertEqual(filter.getValue(), (signed short)100);

    filter.mockValue = 1000;
    assertEqual(filter.getValue(), (signed short)100);

    filter.mockValue = 100;
    assertEqual(filter.getValue(), (signed short)100);
}
