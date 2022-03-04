#ifndef CALIBRATOR_H
#define CALIBRATOR_H

#include "fan.h"

class Calibrator {
  public:
    Calibrator() : m_samples(250), m_avg_period(0uL) {}

    void begin(Fan &fan) {
      Serial.println("Measuring fan speed...");
      m_avg_period = 0ul;
      m_last_rev_time = micros();
      m_samples = 250;
      fan.run(fanISR);
    }

    bool update() {
      if (rev_time) {
        noInterrupts();
        const auto this_rev_time = rev_time;
        rev_time = 0;
        interrupts();
        const auto period = this_rev_time - m_last_rev_time;
        m_last_rev_time = this_rev_time;
        m_avg_period = (15*m_avg_period + period + 8) / 16;
        --m_samples;
      }
      return m_samples <= 0;
    }

    unsigned long fanPeriod() const { return m_avg_period; }

    static float pixelFrequency(unsigned long period, uint16_t pattern_size) {
      const auto freq = 1000000ul * 100ul / period;
      const auto rpm = (60 * freq + 50) / 100;
      Serial.print(F("  Revolution time: "));
      Serial.print(period);
      Serial.println(F(" us (average)"));
      Serial.print(F("  Speed:           "));
      Serial.print(rpm);
      Serial.println(F(" RPM"));
      
      const float pixel_freq = 1.0e6 * pattern_size / period;
      Serial.print(F("For "));
      Serial.print(pattern_size);
      Serial.print(F(" pixels per revolution, PixelTimer must run at "));
      Serial.print(pixel_freq);
      Serial.println(F(" Hz."));
      return pixel_freq;
    }

  private:
    // During calibration, this ISR notes the time (in microseconds)
    // for each revolution.
    static void fanISR() {
      half_rev = !half_rev;
      if (half_rev) return;
      rev_time = micros();
    }

    static bool half_rev;
    static volatile unsigned long rev_time;

    int m_samples;
    unsigned long m_avg_period;
    unsigned long m_last_rev_time;
};

#endif
