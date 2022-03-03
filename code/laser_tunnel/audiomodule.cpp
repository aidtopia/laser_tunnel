#include <Arduino.h>
#include "audiomodule.h"

Audio::Message::Message() :
  m_buf{START, VERSION, LENGTH, 0, FEEDBACK, 0, 0, 0, 0, END},
  m_length(0) {}

void Audio::Message::set(MsgID msgid, uint16_t param, Feedback feedback) {
  // Note that we're filling in just the bytes that change.  We rely
  // on the framing bytes set when the buffer was first initialized.
  m_buf[3] = msgid;
  m_buf[4] = feedback;
  m_buf[5] = (param >> 8) & 0xFF;
  m_buf[6] = (param     ) & 0xFF;
  const uint16_t checksum = ~sum() + 1;
  m_buf[7] = (checksum >> 8) & 0xFF;
  m_buf[8] = (checksum     ) & 0xFF;
  m_length = 10;
}

bool Audio::Message::isValid() const {
  if (m_length == 8 && m_buf[7] == END) return true;
  if (m_length != 10) return false;
  const uint16_t checksum = combine(m_buf[7], m_buf[8]);
  return sum() + checksum == 0;
}

// Returns true if the byte `b` completes a message.
bool Audio::Message::receive(uint8_t b) {
  switch (m_length) {
    default:
      // `m_length` is out of bounds, so start fresh.
      m_length = 0;
      /* FALLTHROUGH */
    case 0: case 1: case 2: case 9:
      // These bytes must always match the template.
      if (b == m_buf[m_length]) { ++m_length; return m_length == 10; }
      // No match; try to resync.
      if (b == START) { m_length = 1; return false; }
      m_length = 0;
      return false;
    case 7:
      // If there's no checksum, the message may end here.
      if (b == END) { m_length = 8; return true; }
      /* FALLTHROUGH */
    case 3: case 4: case 5: case 6: case 8:
      // These are the payload bytes we care about.
      m_buf[m_length++] = b;
      return false;
  }
}

uint16_t Audio::Message::sum() const {
  uint16_t s = 0;
  for (int i = 1; i <= LENGTH; ++i) {
    s += m_buf[i];
  }
  return s;
}

void AdvancedAudioEventHandler::onMessageReceived(const Audio::Message &msg) {
  switch (msg.getMessageID()) {
    case 0x3A: {
      const auto mask = msg.getParamLo();
      if (mask & 0x01) onDeviceInserted(DEV_USB);
      if (mask & 0x02) onDeviceInserted(DEV_SDCARD);
      if (mask & 0x04) onDeviceInserted(DEV_AUX);
      return;
    }
    case 0x3B: {
      const auto mask = msg.getParamLo();
      if (mask & 0x01) onDeviceRemoved(DEV_USB);
      if (mask & 0x02) onDeviceRemoved(DEV_SDCARD);
      if (mask & 0x04) onDeviceRemoved(DEV_AUX);
      return;
    }
    case 0x3C: return onFinishedFile(DEV_USB, msg.getParam());
    case 0x3D: return onFinishedFile(DEV_SDCARD, msg.getParam());
    case 0x3E: return onFinishedFile(DEV_FLASH, msg.getParam());

    // Initialization complete
    case 0x3F: {
      uint16_t devices = 0;
      const auto mask = msg.getParamLo();
      if (mask & 0x01) devices = devices | (1u << DEV_USB);
      if (mask & 0x02) devices = devices | (1u << DEV_SDCARD);
      if (mask & 0x04) devices = devices | (1u << DEV_AUX);
      if (mask & 0x10) devices = devices | (1u << DEV_FLASH);
      return onInitComplete(devices);
    }

    case 0x40: return onError(static_cast<ErrorCode>(msg.getParamLo()));
    
    // ACK
    case 0x41:
      return onAck();

    // Query responses
    case 0x42: {
      // Only Flyron documents this response to the status query.
      // The DFPlayer Mini always seems to report SDCARD even when
      // the selected and active device is USB, so maybe it uses
      // the high byte to signal something else?  Catalex also
      // always reports the SDCARD, but it only has an SDCARD.
      Device device = DEV_SLEEP;
      switch (msg.getParamHi()) {
        case 0x01: device = DEV_USB;     break;
        case 0x02: device = DEV_SDCARD;  break;
      }
      ModuleState state = MS_ASLEEP;
      switch (msg.getParamLo()) {
        case 0x00: state = MS_STOPPED;  break;
        case 0x01: state = MS_PLAYING;  break;
        case 0x02: state = MS_PAUSED;   break;
      }
      return onStatus(device, state);
    }
    case 0x43: return onVolume(msg.getParamLo());
    case 0x44: return onEqualizer(static_cast<Equalizer>(msg.getParamLo()));
    case 0x45: return onPlaybackSequence(static_cast<Sequence>(msg.getParamLo()));
    case 0x46: return onFirmwareVersion(msg.getParam());
    case 0x47: return onDeviceFileCount(DEV_USB, msg.getParam());
    case 0x48: return onDeviceFileCount(DEV_SDCARD, msg.getParam());
    case 0x49: return onDeviceFileCount(DEV_FLASH, msg.getParam());
    case 0x4B: return onCurrentTrack(DEV_USB, msg.getParam());
    case 0x4C: return onCurrentTrack(DEV_SDCARD, msg.getParam());
    case 0x4D: return onCurrentTrack(DEV_FLASH, msg.getParam());
    case 0x4E: return onFolderTrackCount(msg.getParam());
    case 0x4F: return onFolderCount(msg.getParam());
    default: break;
  }
}

void AdvancedAudioEventHandler::onTimedOut() { onError(EC_TIMEDOUT); }

#ifndef NDEBUG
void DebugAudioEventHandler::onMessageSent(const Audio::Message &msg) {
  Serial.print(F("Sent:     "));
  printMessageBytes(msg);
  Serial.println();
  AdvancedAudioEventHandler::onMessageSent(msg);
}

void DebugAudioEventHandler::onMessageReceived(const Audio::Message &msg) {
  Serial.print(F("Received: "));
  printMessageBytes(msg);
  Serial.println();
  AdvancedAudioEventHandler::onMessageReceived(msg);
}

void DebugAudioEventHandler::onTimedOut() {
  Serial.println(F("Timed out."));
  AdvancedAudioEventHandler::onTimedOut();
}

void DebugAudioEventHandler::onAck() {
  Serial.println(F("ACK"));
}

void DebugAudioEventHandler::onCurrentTrack(Device device, uint16_t file_index) {
  printDeviceName(device);
  Serial.print(F(" current file index: "));
  Serial.println(file_index);
}

void DebugAudioEventHandler::onDeviceInserted(Device src) {
  Serial.print(F("Device inserted: "));
  printDeviceName(src);
  Serial.println();
}

void DebugAudioEventHandler::onDeviceRemoved(Device src) {
  printDeviceName(src);
  Serial.println(F(" removed."));
}

void DebugAudioEventHandler::onEqualizer(Equalizer eq) {
  Serial.print(F("Equalizer: "));
  printEqualizerName(eq);
  Serial.println();
}

void DebugAudioEventHandler::onError(Audio::ErrorCode code) {
  Serial.print(F("Error "));
  Serial.print(code);
  Serial.print(F(": "));
  switch (code) {
    case 0x00: Serial.println(F("Unsupported command")); break;
    case 0x01: Serial.println(F("Module busy or no sources available")); break;
    case 0x02: Serial.println(F("Module sleeping")); break;
    case 0x03: Serial.println(F("Serial communication error")); break;
    case 0x04: Serial.println(F("Bad checksum")); break;
    case 0x05: Serial.println(F("File index out of range")); break;
    case 0x06: Serial.println(F("Track not found")); break;
    case 0x07: Serial.println(F("Insertion error")); break;
    case 0x08: Serial.println(F("SD card error")); break;
    case 0x0A: Serial.println(F("Entered sleep mode")); break;
    case 0x100: Serial.println(F("Timed out")); break;
    default:   Serial.println(F("Unknown error code")); break;
  }
}

void DebugAudioEventHandler::onDeviceFileCount(Device device, uint16_t count) {
  printDeviceName(device);
  Serial.print(F(" file count: "));
  Serial.println(count);
}

// Note that this hook receives a file index, even if the track
// was started using something other than its file index.
//
// The module sometimes sends these multiple times in quick
// succession.
//
// This hook does not trigger when the playback is stopped, only
// when a track finishes playing on its own.
//
// This hook does not trigger when an inserted track finishes.
// If you need to know that, you can try watching for a brief
// blink on the BUSY pin of the DF Player Mini.
void DebugAudioEventHandler::onFinishedFile(Device device, uint16_t file_index) {
  Serial.print(F("Finished playing file: "));
  printDeviceName(device);
  Serial.print(F(" "));
  Serial.println(file_index);
}

void DebugAudioEventHandler::onFirmwareVersion(uint16_t version) {
  Serial.print(F("Firmware Version: "));
  Serial.println(version);
}

void DebugAudioEventHandler::onFolderCount(uint16_t count) {
  Serial.print(F("Folder count: "));
  Serial.println(count);
}

void DebugAudioEventHandler::onFolderTrackCount(uint16_t count) {
  Serial.print(F("Folder track count: "));
  Serial.println(count);
}

void DebugAudioEventHandler::onInitComplete(uint16_t devices) {
  Serial.print(F("Hardware initialization complete.  Device(s) online:"));
  if (devices & (1u << DEV_SDCARD)) Serial.print(F(" SD Card"));
  if (devices & (1u << DEV_USB))    Serial.print(F(" USB"));
  if (devices & (1u << DEV_AUX))    Serial.print(F(" AUX"));
  if (devices & (1u << DEV_FLASH))  Serial.print(F(" Flash"));
  Serial.println();
}

void DebugAudioEventHandler::onMessageInvalid() {
  Serial.println(F("Invalid message received."));
}

void DebugAudioEventHandler::onPlaybackSequence(Audio::Sequence seq) {
  Serial.print(F("Playback sequence: "));
  printSequenceName(seq);
  Serial.println();
}

void DebugAudioEventHandler::onStatus(Device device, ModuleState state) {
  Serial.print(F("Device "));
  printDeviceName(device);
  Serial.print(F(" status: "));
  printModuleStateName(state);
  Serial.println();
}

void DebugAudioEventHandler::onVolume(uint8_t volume) {
  Serial.print(F("Volume: "));
  Serial.println(volume);
}

void DebugAudioEventHandler::printDeviceName(Audio::Device src) {
  switch (src) {
    case DEV_USB:    Serial.print(F("USB")); break;
    case DEV_SDCARD: Serial.print(F("SD Card")); break;
    case DEV_AUX:    Serial.print(F("AUX")); break;
    case DEV_SLEEP:  Serial.print(F("SLEEP (does this make sense)")); break;
    case DEV_FLASH:  Serial.print(F("FLASH")); break;
    default:         Serial.print(F("Unknown Device")); break;
  }
}

void DebugAudioEventHandler::printEqualizerName(Audio::Equalizer eq) {
  switch (eq) {
    case EQ_NORMAL:    Serial.print(F("Normal"));    break;
    case EQ_POP:       Serial.print(F("Pop"));       break;
    case EQ_ROCK:      Serial.print(F("Rock"));      break;
    case EQ_JAZZ:      Serial.print(F("Jazz"));      break;
    case EQ_CLASSICAL: Serial.print(F("Classical")); break;
    case EQ_BASS:      Serial.print(F("Bass"));      break;
    default:           Serial.print(F("Unknown EQ")); break;
  }
}

void DebugAudioEventHandler::printModuleStateName(Audio::ModuleState state) {
  switch (state) {
    case MS_STOPPED: Serial.print(F("Stopped")); break;
    case MS_PLAYING: Serial.print(F("Playing")); break;
    case MS_PAUSED:  Serial.print(F("Paused"));  break;
    case MS_ASLEEP:  Serial.print(F("Asleep"));  break;
    default:         Serial.print(F("???"));     break;
  }
}

void DebugAudioEventHandler::printSequenceName(Audio::Sequence seq) {
  switch (seq) {
    case SEQ_LOOPALL:    Serial.print(F("Loop All")); break;
    case SEQ_LOOPFOLDER: Serial.print(F("Loop Folder")); break;
    case SEQ_LOOPTRACK:  Serial.print(F("Loop Track")); break;
    case SEQ_RANDOM:     Serial.print(F("Random")); break;
    case SEQ_SINGLE:     Serial.print(F("Single")); break;
    default:             Serial.print(F("???")); break;
  }
}

void DebugAudioEventHandler::printMessageBytes(const Audio::Message &msg) {
  const auto *buf = msg.getBuffer();
  const auto len = msg.getLength();
  for (int i = 0; i < len; ++i) {
    Serial.print(F(" "));
    Serial.print(buf[i], HEX);
  }
}

#endif
