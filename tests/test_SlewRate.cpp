#include <AUnit.h>
#include "MockReader.h"
#include "../src/evafSlewRate.h"

test(SlewRate_MaxStepConstraint)
{
    evaf::SlewRate<evaf_test::MockReader, 50> filter(50);

    filter.mockValue = 0;
    assertEqual(filter.getValue(), (signed short)0);

    filter.mockValue = 200;

    assertEqual(filter.getValue(), (signed short)50);
    assertEqual(filter.getValue(), (signed short)100);
    assertEqual(filter.getValue(), (signed short)150);
    assertEqual(filter.getValue(), (signed short)200);
    assertEqual(filter.getValue(), (signed short)200);
}
