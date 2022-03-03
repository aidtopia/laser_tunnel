#ifndef SOUNDFX_H
#define SOUNDFX_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "audiomodule.h"
#include "pins.h"

#if 1
#define SOUNDFX_BASE_CLASS AudioEventHandler
#else
#define SOUNDFX_BASE_CLASS DebugAudioEventHandler
#endif

class SoundFX : public SOUNDFX_BASE_CLASS {
  public:
    SoundFX(int rx_pin, int tx_pin, int busy_pin) :
      m_serial(rx_pin, tx_pin),
      m_busy(busy_pin),
      m_module(m_serial),
      m_file_count(0),
      m_file_playing(0) {}

    void begin() {
      m_busy.begin();
      m_module.begin(this);
      m_module.reset();
    }
    void update() { m_module.update(); }

    // The Track values are the required file indexes for the sound
    // effects.  User many include any number of sounds, as long as
    // they are arranged in this order.
    enum Track : uint16_t { NONE=0, STARTLE=1, AMBIENT=2, EMERGENCY=3 };

    bool has(Track track) const {
      const auto file_index = static_cast<uint16_t>(track);
      return m_file_count >= file_index;
    }

    bool isBusy() const {
      return m_file_playing == STARTLE || m_busy.read() == LOW;
    }

    void play(Track track) {
      if (track == NONE || !has(track)) {
        if (isBusy()) m_module.stop();
        m_file_playing = 0;
        return;
      }
      const auto file_index = static_cast<uint16_t>(track);
      m_module.playFile(file_index, Audio::NO_FEEDBACK);
      m_file_playing = file_index;
    }

    void stop() { play(NONE); }

    void onDeviceInserted(Device src) override {
      SOUNDFX_BASE_CLASS::onDeviceInserted(src);
      if (src == Audio::DEV_SDCARD) {
        m_file_count = 0;
        m_file_playing = 0;
        m_module.queryFileCount(Audio::DEV_SDCARD);
      }
    }
    
    void onDeviceFileCount(Device src, uint16_t count) override {
      SOUNDFX_BASE_CLASS::onDeviceFileCount(src, count);
      if (src == Audio::DEV_SDCARD) {
        m_file_count = count;
      }
    }

    void onDeviceRemoved(Device src) override {
      SOUNDFX_BASE_CLASS::onDeviceRemoved(src);
      if (src == Audio::DEV_SDCARD) {
        m_file_count = 0;
        m_file_playing = 0;
      }
    }

    void onInitComplete(uint16_t devices) override {
      SOUNDFX_BASE_CLASS::onInitComplete(devices);
      // Note that onInitComplete comes after a reset and also
      // after a select source command.  (I think that's because
      // the reset implicitly selects a source.)  Do not respond
      // to this by sending a source select or you'll get stuck
      // in an endless loop.
      if (devices & (1u << Audio::DEV_SDCARD)) {
        m_module.queryFileCount(Audio::DEV_SDCARD);
      }
    }

    void onFinishedFile(Device device, uint16_t file_index) override {
      SOUNDFX_BASE_CLASS::onFinishedFile(device, file_index);
      if (device == Audio::DEV_SDCARD && file_index == m_file_playing) {
        m_file_playing = 0;
      }
      // The ambient sound should loop.  In theory, the audio module
      // can do that, but it doesn't seem to work on all models, at
      // least, not with short clips.  So if the ambient track is
      // the one that just finished, we'll restart it.
      if (file_index == AMBIENT) play(AMBIENT);
    }

  private:
    SoftwareSerial m_serial;
    DigitalInputPin m_busy;
    AudioModule<SoftwareSerial> m_module;
    uint16_t m_file_count;
    uint16_t m_file_playing;
};

#undef SOUNDFX_BASE_CLASS

#endif
