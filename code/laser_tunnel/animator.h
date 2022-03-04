#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <Arduino.h>
#include "patternbuffer.h"

class Animator {
  public:
    Animator() : m_frame(0) {}

    void begin(PatternBuffer &pattern) {
      m_frame = 0;
      onFrame(pattern, 0);
    }

    void update(volatile bool &rev_flag, PatternBuffer &pattern) {
      noInterrupts();
      const bool need_frame = rev_flag;
      rev_flag = false;
      interrupts();
      if (need_frame) onFrame(pattern, ++m_frame);
    }
    
  private:
    virtual void onFrame(PatternBuffer &, unsigned) {}

    unsigned m_frame;
};

class RotaryCorruption : public Animator {
  public:
    RotaryCorruption() : Animator() {}

  private:
    virtual void onFrame(PatternBuffer &pattern, unsigned frame) {
      if (frame == 0) {
        pattern.setTestPattern();
        return;
      }
      pattern.rotate(-1);
      pattern.togglePixel(random(pattern.size()));
    }
};

#endif
