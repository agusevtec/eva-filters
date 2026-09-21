#include <Arduino.h>
#include <evaTac.h>
#include <evaHeartbeat.h>

// Подключаем заголовочные файлы фильтров
#include <evafExpAdaptiveAverage.h>
#include <evafMedian.h>
#include <evafOpenClose.h>
#include <evafMinmax.h>
#include <evafSimpleAverage.h>
#include <evafExponentialAverage.h>
#include <evafSlewRate.h>

using namespace eva;

// ============================================================================
// Глобальная переменная для хранения исходного сырого сигнала
// ============================================================================
volatile signed short g_rawSignal = 0;

// ============================================================================
// Псевдо-ридер, читающий данные из глобальной переменной
// ============================================================================
class GlobalVar
{
public:
    signed short getValue() const
    {
        return g_rawSignal;
    }
    bool isValid()
    {
      return true;
    }
};

// ============================================================================
// Имитатор/Генератор сигнала
// ============================================================================
class SignalGenerator : public Heartbeat
{
private:
    enum Mode { MODE_ZERO, MODE_SAWTOOTH, MODE_SINE };

    Mode mMode = MODE_SAWTOOTH;
    unsigned long mLastModeSwitchMs = 0;
    unsigned long mStepCounter = 0;

public:
    SignalGenerator() : Heartbeat(100) {} // Обновление каждые 10 мс

protected:
    void onHeartbeat() override
    {
        unsigned long now = millis();

        // Каждые 4 секунды случайно меняем режим генерации
        if (now - mLastModeSwitchMs > 4000)
        {
            mLastModeSwitchMs = now;
            mMode = static_cast<Mode>(random(0, 3));
        }

        mStepCounter++;

        signed short baseSignal = 0;

        switch (mMode)
        {
            case MODE_SAWTOOTH:
                // Пилообразный сигнал от -800 до 800
                baseSignal = -800 + (signed short)((mStepCounter * 20) % 1600);
                break;

            case MODE_SINE:
                // Синусоида с амплитудой +-700
                baseSignal = (signed short)(700.0f * sin(mStepCounter * 0.05f));
                break;

            case MODE_ZERO:
            default:
                baseSignal = 0;
                break;
        }

        // В 3% случаев подмешиваем разовый резкий спайк (импульсный шум)
        if (random(0, 100) < 3)
        {
            baseSignal += (random(0, 2) == 0 ? 900 : -900);
        }

        g_rawSignal = constrain(baseSignal, -1000, 1000);
    }
};

SignalGenerator generator;

// ============================================================================
// Цепи фильтрации (-декораторы над GlobalVar)
// ============================================================================

// 1. Адаптивный сглаживающий фильтр (10ms - 150ms)
evaf::ExpAdaptiveAverage<GlobalVar, 10, 150> adaptiveSmoothFilter;

// 2. Медианный фильтр (окно = 5 элементов)
evaf::Median<GlobalVar, 3> medianFilter;

// 3. Морфологический MinMax фильтр (N = 3, буфер N*N = 9)
evaf::OpenClose<GlobalVar, 7> opencloseFilter;

// 3. Морфологический MinMax фильтр (N = 3, буфер N*N = 9)
evaf::Minmax<GlobalVar, 2> minmaxFilter;

// 4. Скользящее среднее (окно = 8 элементов)
evaf::SimpleAverage<GlobalVar, 3> slidingWindowFilter;

// 5. Экспоненциальный фильтр EMA (alpha = 150 / 1000)
evaf::ExponentialAverage<GlobalVar, 500> exponentialSmoothFilter;

// 6. Ограничитель скорости изменения (макс. шаг 30 ед. за тик 10ms)
evaf::SlewRate<GlobalVar, 300> slewRateFilter;

// ============================================================================
// Регулярный вывод данных 10 раз в секунду (период 100 мс = 10 Гц)
// ============================================================================
class SerialPrinter : public Heartbeat
{
public:
    SerialPrinter() : Heartbeat(100) {}

protected:
    void onHeartbeat() override
    {
        // Печать в формате Serial Plotter: сырое значение и результат каждого фильтра
//        Serial.print("-1000 1000 3000 ");
        Serial.print(g_rawSignal);
        // Serial.print(' ');
        // Serial.print(adaptiveSmoothFilter.getValue());
        // Serial.print(' ');
        // Serial.print(medianFilter.getValue());
        Serial.print(' ');
        Serial.print(minmaxFilter.getValue());
        Serial.print(' ');
        Serial.print(opencloseFilter.getValue());
        // Serial.print(' ');
        // Serial.print(slidingWindowFilter.getValue());
        // Serial.print(' ');
        // Serial.print(exponentialSmoothFilter.getValue());
        // Serial.print(' ');
        // Serial.print(slewRateFilter.getValue());
        Serial.println();
    }
};


// ============================================================================
// Setup и Main Loop
// ============================================================================
void setup()
{
    Serial.begin(9600);
    static SerialPrinter printer;
    randomSeed(analogRead(0));
}

void loop()
{
    // Система автоматически вызывает tick() для генератора, принтера и фильтров
    eva::tac();
}
