#include <AUnit.h>

using namespace aunit;

void setup()
{
  aunit::TestRunner::setVerbosity(aunit::Verbosity::kAssertionFailed | aunit::Verbosity::kTestRunSummary);
}

void loop()
{
  aunit::TestRunner::run();
}
