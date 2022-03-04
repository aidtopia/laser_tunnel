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
      m_file_playing(0),
      m_file_count(0),
      m_durations() {}

    void begin() {
      m_busy.begin();
      m_module.begin(this);
      m_module.reset();
    }
    void update() { m_module.update(); }

    // The Track values are the required file indexes for the sound
    // effects.  User many include any number of sounds, as long as
    // the first ones are arranged in this order.
    enum Track : uint16_t {
      NONE,
      STARTLE,
      AMBIENT,
      EMERGENCY,
      SLOTS
    };

    bool has(Track track) const {
      const auto file_index = static_cast<uint16_t>(track);
      return m_file_count >= file_index;
    }

    // Returns the duration of the requested track in milliseconds,
    // or 0 if it's not yet known.
    unsigned long duration(Track track) const {
      if (track <= NONE || SLOTS <= track) return 0uL;
      const auto file_index = static_cast<uint16_t>(track);
      if (file_index > m_file_count) return 0uL;
      return m_durations[file_index];
    }

    Track currentTrack() const {
      return static_cast<Track>(m_file_playing);
    }

    void play(Track track) {
      if (track == NONE || !has(track)) {
        if (m_file_playing != 0) {
          m_module.stop();
          m_file_playing = 0;
        }
        return;
      }
      const auto file_index = static_cast<uint16_t>(track);

      // If we don't know the duration of this track, stash
      // the start time in the NONE slot of m_durations.
      if (file_index < SLOTS) {
        if (m_durations[file_index] == 0) {
          m_durations[NONE] = millis();
        }
      }
      
      m_module.playFile(file_index, Audio::NO_FEEDBACK);
      m_file_playing = file_index;
    }

    void stop() { play(NONE); }

    void onDeviceInserted(Device src) override {
      SOUNDFX_BASE_CLASS::onDeviceInserted(src);
      if (src == Audio::DEV_SDCARD) {
        clearCache();
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
        m_file_playing = 0;
        clearCache();
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
        clearCache();
        m_module.queryFileCount(Audio::DEV_SDCARD);
      }
    }

    void onFinishedFile(Device device, uint16_t file_index) override {
      SOUNDFX_BASE_CLASS::onFinishedFile(device, file_index);
      if (device != Audio::DEV_SDCARD) return;
      if (file_index != m_file_playing) return;

      // If we were timing this track, record its duration.
      if (m_file_playing <= SLOTS && m_durations[m_file_playing] == 0uL) {
        m_durations[m_file_playing] = millis() - m_durations[NONE];
        m_durations[NONE] = 0;
      }

      m_file_playing = 0;
    }

  private:
    void clearCache() {
      m_file_count = 0;
      for (auto &d : m_durations) d = 0uL;
    }

    SoftwareSerial m_serial;
    DigitalInputPin m_busy;
    AudioModule<SoftwareSerial> m_module;
    uint16_t m_file_playing;

    // Cached metadata about the audio tracks.
    uint16_t m_file_count;
    unsigned long m_durations[SLOTS];
};

#undef SOUNDFX_BASE_CLASS

#endif
