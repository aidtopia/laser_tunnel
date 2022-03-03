#ifndef SUPPRESSOR_H
#define SUPPRESSOR_H

#include <Arduino.h>
#include "laser.h"
#include "pins.h"
#include "timeout.h"

class Suppressor {
  public:
    Suppressor(int high_pin, int low_pin, int time_pin) :
      m_high(high_pin), m_low(low_pin), m_time_pin(time_pin),
      m_timer() {}

    void begin() {
      m_high.begin();
      m_low.begin(INPUT_PULLUP);
    }

    void update(Laser &laser) {
      if (m_high.read() == HIGH || m_low.read() == LOW) {
        if (!m_timer.active()) {
          laser.disable();
          Serial.print(F("Suppressing at least "));
          Serial.print(duration());
          Serial.println(F(" ms"));
        }
        m_timer.set(duration());
      }
      
      if (m_timer.active() && m_timer.expired()) {
        m_timer.cancel();
        laser.enable();
      }
    }

  private:
    unsigned long duration() const {
      return map(analogRead(m_time_pin), 1023, 0, 3, 30)*1000;
    }

    DigitalInputPin m_high;
    DigitalInputPin m_low;
    int m_time_pin;
    Timeout<MillisClock> m_timer;
};

#endif
