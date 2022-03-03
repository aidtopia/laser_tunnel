#ifndef SOUNDFX_H
#define SOUNDFX_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "audiomodule.h"
#include "pins.h"

class SoundFX : public AdvancedAudioEventHandler {
  public:
    SoundFX(int rx_pin, int tx_pin, int busy_pin) :
      m_serial(rx_pin, tx_pin),
      m_busy(busy_pin),
      m_module(m_serial) {}

    void begin() {
      m_busy.begin();
      m_module.begin(this);
    }
    void update() { m_module.update(); }

    void loopAmbientSound() {
      // `loopFile()` doesn't seem to work for short clips, so
      // we'll call `playFile()` and loop it ourselves.
      m_module.playFile(2);
    }

    void playStartleSound() { m_module.playFile(1); }

    bool isBusy() const { return m_busy.read() == LOW; }

    void stop() { m_module.stop(); }

    void onInitComplete(uint16_t devices) override {
      if (devices & (1u << Audio::DEV_SDCARD)) {
        m_module.queryFileCount(Audio::DEV_SDCARD);
      }
    }

    void onFinishedFile(Device device, uint16_t /*file_index*/) override {
      // Either the ambient sound finished and we need to play it again,
      // or the startle sound finished and we need to switch back to
      // the ambient sound.
      if (device == Audio::DEV_SDCARD) {
        loopAmbientSound();
      }
    }

  private:
    SoftwareSerial m_serial;
    DigitalInputPin m_busy;
    AudioModule<SoftwareSerial> m_module;
};

#endif
