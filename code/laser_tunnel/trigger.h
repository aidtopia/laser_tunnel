#ifndef TRIGGER_H
#define TRIGGER_H

#include <Arduino.h>
#include "pins.h"

class Trigger {
  public:
    Trigger(int high_pin, int low_pin) : m_high(high_pin), m_low(low_pin) {}

    void begin() {
      m_high.begin();
      m_low.begin(INPUT_PULLUP);
    }

    bool read() const {
      return m_high.read() == HIGH || m_low.read() == LOW;
    }

  private:
    DigitalInputPin m_high;
    DigitalInputPin m_low;
};

#endif
