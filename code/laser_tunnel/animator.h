#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <Arduino.h>
#include "patternbuffer.h"

typedef void (*Animation)(PatternBuffer &pattern, unsigned frame);

class Animator {
  public:
    Animator() : m_frame(0), m_animation(nullptr) {}

    void setAnimation(Animation animation) {
      m_frame = 0;
      m_animation = animation;
    }

    void update(volatile bool &rev_flag, PatternBuffer &pattern) {
      if (m_animation == nullptr) return;
      noInterrupts();
      const bool need_frame = rev_flag;
      rev_flag = false;
      interrupts();
      if (need_frame) (*m_animation)(pattern, m_frame++);
    }
    
  private:
    unsigned m_frame;
    Animation m_animation;
};

void Glitch(PatternBuffer &pattern, unsigned frame) {
  static unsigned glitch_frame = 0;
  static unsigned restore_frame = 0;

  if (frame == 0) {
    pattern.setTestPattern();
    restore_frame = 0;
  }

  if (frame == restore_frame) {
    pattern.setRotation(0);
    glitch_frame = frame + random(10, 90);
  } else if (frame == glitch_frame) {
    pattern.setRotation(10*random(-10, 10));
    restore_frame = frame + random(3, 25);
  }
}


void RadialSeeds(PatternBuffer &pattern, unsigned frame) {
  static uint8_t m_seeds[4];

  if (frame == 0) {
    pattern.clear();
    for (auto &s : m_seeds) s = random(256);
  }

  for (int i = 0; i < 4; ++i) {
    const auto seed = m_seeds[i];
    const auto offset = (i+1)*frame/8;
    pattern.setPixel(seed + offset);
    pattern.setPixel(seed - offset);
  }
}

void RotaryCorruption(PatternBuffer &pattern, unsigned /*frame*/) {
  pattern.rotate(-1);
  pattern.togglePixel(random(pattern.size()));
}

void WaxOn(PatternBuffer &pattern, unsigned frame) {
  if (frame == 0) {
    pattern.clear();
    pattern.setRotation(0);
    return;
  }
  pattern.setPixel(2*frame);
  pattern.setPixel(2*frame+1);
}

void WaxOff(PatternBuffer &pattern, unsigned frame) {
  pattern.clearPixel(255 - (2*frame));
  pattern.clearPixel(255 - (2*frame+1));
}


void Composite(PatternBuffer &pattern, unsigned frame) {
  frame = frame % 628;
  if (frame <= 128) return WaxOn(pattern, frame);
  if (frame <= 500) return RotaryCorruption(pattern, frame);
  if (frame <= 628) return WaxOff(pattern, frame);
}

#endif
