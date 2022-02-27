#ifndef LASER_H
#define LASER_H

#include <Arduino.h>
#include "pins.h"

class Laser {
  public:
    explicit Laser(int pin) : m_pin(pin), m_enabled(false) {}

    void begin() { enable(); off(); }
    void enable() { m_enabled = true; }
    void disable() {
      noInterrupts();
      m_pin.clear();
      m_enabled = false;
      interrupts();
    }

    void on() { if (m_enabled) m_pin.set(); }
    void off() { m_pin.clear(); }

  private:
    DigitalOutputPin m_pin;
    bool m_enabled;
};

#endif
