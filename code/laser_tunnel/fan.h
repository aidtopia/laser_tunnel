#ifndef FAN_H
#define FAN_H

#include <Arduino.h>
#include "aidassert.h"
#include "pins.h"

class Fan {
  public:
    Fan(int tach_pin, int pwm_pin) :
      m_tach(tach_pin), m_pwm(pwm_pin) {}

    void begin() {
      // The tachometer output from the fan must be connected
      // to a pin that can generate external interrupts.
      ASSERT(digitalPinToInterrupt(m_tach) != NOT_AN_INTERRUPT);
      m_tach.begin(INPUT_PULLUP);
      m_pwm.begin(LOW);
    }

    void run(void (*pfn_isr)() = nullptr) {
      const auto interrupt = digitalPinToInterrupt(m_tach);
      if (pfn_isr == nullptr) detachInterrupt(interrupt);
      else attachInterrupt(interrupt, pfn_isr, FALLING);
      m_pwm.set();
    }
    void stop() { m_pwm.clear(); }

  private:
    DigitalInputPin m_tach;
    DigitalOutputPin m_pwm;
};

#endif
