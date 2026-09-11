#pragma once

    class MockReader
    {
    public:
        signed short mockValue = 0;
        int dummyPin = -1;

        MockReader() = default;
        explicit MockReader(int pin) : dummyPin(pin) {}

        signed short getValue()
        {
            return mockValue;
        }
        bool isValid()
        {
            return true;
        }
    };
