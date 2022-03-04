#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <Arduino.h>
#include "patternbuffer.h"

class Animation {
  public:
    virtual void onFrame(PatternBuffer &pattern, unsigned frame) = 0;
};

class Animator {
  public:
    Animator() : m_frame(0), m_animation(nullptr) {}

    void setAnimation(Animation *animation) {
      m_frame = 0;
      m_animation = animation;
    }

    void update(volatile bool &rev_flag, PatternBuffer &pattern) {
      if (m_animation == nullptr) return;
      noInterrupts();
      const bool need_frame = rev_flag;
      rev_flag = false;
      interrupts();
      if (need_frame) m_animation->onFrame(pattern, m_frame++);
    }
    
  private:
    virtual void onFrame(PatternBuffer &, unsigned) {}

    unsigned m_frame;
    Animation *m_animation;
};

class Glitch : public Animation {
  public:
    void onFrame(PatternBuffer &pattern, unsigned frame) override {
      if (frame == 0) {
        pattern.setTestPattern();
        m_restore_frame = 0;
      }

      if (frame == m_restore_frame) {
        pattern.setRotation(0);
        m_glitch_frame = frame + random(10, 150);
      } else if (frame == m_glitch_frame) {
        pattern.setRotation(7*random(-10, 10));
        m_restore_frame = frame + random(3, 25);
      }
    }

  private:
    unsigned m_glitch_frame = 0;
    unsigned m_restore_frame = 0;
};

class RadialSeeds : public Animation {
  public:
    void onFrame(PatternBuffer &pattern, unsigned frame) override {
      if (frame == 0) {
        pattern.clear();
        for (auto &s : m_seeds) s = random(256);
      }

      for (int i = 0; i < 4; ++i) {
        const auto seed = m_seeds[i];
        const auto offset = (i+1)*frame/4;
        pattern.setPixel(seed + offset);
        pattern.setPixel(seed - offset);
      }
    }
    
  private:
    uint8_t m_seeds[4];
};

class RotaryCorruption : public Animation {
  public:
    void onFrame(PatternBuffer &pattern, unsigned frame) override {
      if (frame == 0) {
        pattern.setTestPattern();
        return;
      }
      pattern.rotate(-1);
      pattern.togglePixel(random(pattern.size()));
    }
};

class WaxOn : public Animation {
  public:
    void onFrame(PatternBuffer &pattern, unsigned frame) override {
      if (frame == 0) {
        pattern.clear();
        pattern.setRotation(0);
      }
      pattern.togglePixel(2*frame);
      pattern.togglePixel(2*frame+1);
    }
};

#endif
