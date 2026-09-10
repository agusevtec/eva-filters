#include <AUnit.h>
#include "MockReader.h"
#include "../src/evafSlidingWindow.h"

test(SlidingWindow_Averaging)
{
    evaf::SlidingWindow<evaf_test::MockReader, 4> filter;

    filter.mockValue = 100;
    filter.getValue();

    filter.mockValue = 200;
    filter.getValue();

    filter.mockValue = 300;
    filter.getValue();

    filter.mockValue = 400;
    assertEqual(filter.getValue(), (signed short)250);
}
