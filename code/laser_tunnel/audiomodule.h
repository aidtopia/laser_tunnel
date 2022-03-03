// AudioModule
// Adrian McCarthy 2018-2022

// A library that works with various serial audio modules,
// like DFPlayer Mini, Catalex, etc.
#ifndef AUDIOMODULE_H
#define AUDIOMODULE_H

#include "timeout.h"

// This is a base class with a bunch of definitions common to
// audio modules and audio event handling.
struct Audio {
  enum Device {
    DEV_USB,      // a storage device connected via USB
    DEV_SDCARD,   // a micro SD card in the TF slot
    DEV_AUX,      // typically a connection to a PC
    DEV_SLEEP,    // a pseudo-device to indicate the module is sleeping
    DEV_FLASH,    // internal flash memory
    // Synonyms used in the datasheets:
    DEV_TF  = DEV_SDCARD, // The SD card slot is sometimes called TF (True Flash)
    DEV_PC  = DEV_AUX,    // The AUX input is typically a PC connection
    DEV_SPI = DEV_FLASH   // The internal flash memory is an SPI device
  };

  enum Equalizer { EQ_NORMAL, EQ_POP, EQ_ROCK, EQ_JAZZ, EQ_CLASSICAL, EQ_BASS };
  enum ModuleState { MS_STOPPED, MS_PLAYING, MS_PAUSED, MS_ASLEEP };
  enum Sequence { SEQ_LOOPALL, SEQ_LOOPFOLDER, SEQ_LOOPTRACK, SEQ_RANDOM, SEQ_SINGLE };

  // These are the message IDs (sometimes called commands) for the
  // messages in the serial protocol for the YX5200 and YX5300 chips.
  enum MsgID : uint8_t {
    // First, the commands
    MID_PLAYNEXT          = 0x01,
    MID_PLAYPREVIOUS      = 0x02,
    MID_PLAYFILE          = 0x03,
    MID_VOLUMEUP          = 0x04,
    MID_VOLUMEDOWN        = 0x05,
    MID_SETVOLUME         = 0x06,
    MID_SELECTEQ          = 0X07,
    MID_LOOPFILE          = 0x08,
    MID_LOOPFLASHTRACK    = MID_LOOPFILE,  // Alternate msg not used
    MID_SELECTSOURCE      = 0x09,
    MID_SLEEP             = 0x0A,
    MID_WAKE              = 0x0B,
    MID_RESET             = 0x0C,
    MID_RESUME            = 0x0D,
    MID_UNPAUSE           = MID_RESUME,
    MID_PAUSE             = 0x0E,
    MID_PLAYFROMFOLDER    = 0x0F,
    MID_VOLUMEADJUST      = 0x10,  // Seems busted, use MID_SETVOLUME
    MID_LOOPALL           = 0x11,
    MID_PLAYFROMMP3       = 0x12,  // "MP3" here refers to name of folder
    MID_INSERTADVERT      = 0x13,
    MID_PLAYFROMBIGFOLDER = 0x14,
    MID_STOPADVERT        = 0x15,
    MID_STOP              = 0x16,
    MID_LOOPFOLDER        = 0x17,
    MID_RANDOMPLAY        = 0x18,
    MID_LOOPCURRENTFILE   = 0x19,
    MID_DISABLEDAC        = 0x1A,
    MID_PLAYLIST          = 0x1B,  // Might not work, unusual message length
    MID_PLAYWITHVOLUME    = 0x1C,  // seems redundant

    // Asynchronous messages from the module
    MID_DEVICEINSERTED    = 0x3A,
    MID_DEVICEREMOVED     = 0x3B,
    MID_FINISHEDUSBFILE   = 0x3C,
    MID_FINISHEDSDFILE    = 0x3D,
    MID_FINISHEDFLASHFILE = 0x3E,

    // Quasi-asynchronous
    MID_INITCOMPLETE      = 0x3F,

    // Basic replies
    MID_ERROR             = 0x40,
    MID_ACK               = 0x41,

    // Queries and their responses
    MID_STATUS            = 0x42,
    MID_VOLUME            = 0x43,
    MID_EQ                = 0x44,
    MID_PLAYBACKSEQUENCE  = 0x45,
    MID_FIRMWAREVERSION   = 0x46,
    MID_USBFILECOUNT      = 0x47,
    MID_SDFILECOUNT       = 0x48,
    MID_FLASHFILECOUNT    = 0x49,
    // no 0x4A?
    MID_CURRENTUSBFILE    = 0x4B,
    MID_CURRENTSDFILE     = 0x4C,
    MID_CURRENTFLASHFILE  = 0x4D,
    MID_FOLDERTRACKCOUNT  = 0x4E,
    MID_FOLDERCOUNT       = 0x4F,

    // We're going to steal an ID for our state machine's use.
    MID_ENTERSTATE        = 0x00
  };

  enum ErrorCode : uint16_t {
    EC_UNSUPPORTED        = 0x00,  // MsgID used is not supported
    EC_NOSOURCES          = 0x01,  // module busy or no sources installed
    EC_SLEEPING           = 0x02,  // module is sleeping
    EC_SERIALERROR        = 0x03,  // serial communication error
    EC_BADCHECKSUM        = 0x04,  // module received bad checksum
    EC_FILEOUTOFRANGE     = 0x05,  // this is the file index
    EC_TRACKNOTFOUND      = 0x06,  // couldn't find track by numeric prefix
    EC_INSERTIONERROR     = 0x07,  // couldn't start ADVERT track
    EC_SDCARDERROR        = 0x08,  // ??
    EC_ENTEREDSLEEP       = 0x0A,  // entered sleep mode??

    // And reserving one for our state machine
    EC_TIMEDOUT           = 0x0100
  };

  // TODO:  Could/should these be part of Message?
  static constexpr uint16_t combine(uint8_t hi, uint8_t lo) {
    return static_cast<uint16_t>(hi << 8) | lo;
  }
  
  static constexpr uint8_t high(uint16_t x) { return x >> 8; }
  static constexpr uint8_t low(uint16_t x)  { return x & 0xFF; }

  // Manages a buffered message with all the protocol details.
  class Message {
    public:
      enum { START = 0x7E, VERSION = 0xFF, LENGTH = 6, END = 0xEF };
      enum Feedback { NO_FEEDBACK = 0x00, FEEDBACK = 0x01 };

      Message();

      void set(MsgID msgid, uint16_t param, Feedback feedback = NO_FEEDBACK);

      const uint8_t *getBuffer() const { return m_buf; }
      int getLength() const { return m_length; }

      bool isValid() const;

      MsgID getMessageID() const { return static_cast<MsgID>(m_buf[3]); }
      uint8_t getParamHi() const { return m_buf[5]; }
      uint8_t getParamLo() const { return m_buf[6]; }
      uint16_t getParam() const  { return combine(m_buf[5], m_buf[6]); }

      // Returns true if the byte `b` completes a message.
      bool receive(uint8_t b);

    private:
      // Sums the bytes used to compute the Message's checksum.
      uint16_t sum() const;

      uint8_t m_buf[10];
      int m_length;
  };
};

class BasicAudioModule;

class BasicAudioEventHandler : public Audio {
  public:
    virtual void onMessageSent(const Message &/*msg*/) {}
    virtual void onMessageReceived(const Message &/*msg*/) {}
    virtual void onTimedOut() {}
};

class BasicAudioModule : public Audio {
  public:
    explicit BasicAudioModule(Stream &stream) :
      m_stream(stream), m_in(), m_out(), m_timeout(), m_handler(nullptr) {}

    virtual void begin(BasicAudioEventHandler *handler = nullptr) {
      m_handler = handler;
    }

    // Call each time through `loop`.
    void update() {
      checkForIncomingMessage();
      checkForTimeout();
    }

    // Reset the audio module.
    // 
    // Resetting causes an unavoidable click on the output.
    void reset() {
      sendCommand(MID_RESET);
      // extend the timeout because reset takes a while
      m_timeout.set(10000);
    }

    // Select a Device to be the current source.
    //
    // Many modules select `DEV_SDCARD` by default, which is usually
    // appropriate, but it's good practice to select it yourself to be
    // certain.
    void selectSource(Device device) {
      switch (device) {
        case DEV_USB:    sendCommand(MID_SELECTSOURCE, 1); break;
        case DEV_SDCARD: sendCommand(MID_SELECTSOURCE, 2); break;
        case DEV_FLASH:  sendCommand(MID_SELECTSOURCE, 5); break;
        default: break;
      }
    }
    
    // Play a file selected by its file system index.
    //
    // If you don't know the file index of the track you want, you should
    // probably use `playTrack` instead.
    //
    // This command can play a track from any folder on the selected source
    // device.  You can use `queryFileCount` to find out how many
    // files are available.
    //
    // Corresponds to playback sequence `SEQ_SINGLE`.
    void playFile(uint16_t file_index) { sendCommand(MID_PLAYFILE, file_index); }

    // Play the next file based on the current file index.
    void playNextFile() { sendCommand(MID_PLAYNEXT); }

    // Play the previous file based on the current file index.
    void playPreviousFile() { sendCommand(MID_PLAYPREVIOUS); }

    // Play a single file repeatedly.
    //
    // Corresponds to playback sequence `SEQ_LOOPTRACK`.
    void loopFile(uint16_t file_index) { sendCommand(MID_LOOPFILE, file_index); }

    // Play all the files on the device, in file index order, repeatedly.
    //
    // Corresponds to playback sequence `SEQ_LOOPALL`.
    void loopAllFiles() { sendCommand(MID_LOOPALL, 1); }

    // Play all the files on the current device in a random order.
    //
    // TBD: Does it repeat once it has played all of them?
    //
    // Corresponds to playback sequence `SEQ_RANDOM`.
    void playFilesInRandomOrder() { sendCommand(MID_RANDOMPLAY); }
    
    // playTrack lets you specify a folder named with two decimal
    // digits, like "01" or "02", and a track whose name begins
    // with three decimal digits like "001.mp3" or
    // "014 Yankee Doodle.wav".
    void playTrack(uint16_t folder, uint16_t track) {
      // Under the hood, there are a couple different command
      // messages to achieve this. We'll automatically select the
      // most appropriate one based on the values.
      if (track < 256) {
        const uint16_t param = (folder << 8) | track;
        sendCommand(MID_PLAYFROMFOLDER, param);
      } else if (folder < 16 && track <= 3000) {
        // For folders with more than 255 tracks, we have this
        // alternative command.
        const uint16_t param = (folder << 12) | track;
        sendCommand(MID_PLAYFROMBIGFOLDER, param);
      }
    }

    // This overload lets you select a track whose file name begins
    // with a three or four decimal digit number, like "001" or "2432".
    // The file must be in a top-level folder named "MP3".  It's
    // recommended that you have fewer than 3000 files in this folder
    // in order to avoid long startup delays as the module searches
    // for the right file.
    //
    // Even though the folder is named "MP3", it may contain .wav
    // files as well.
    void playTrack(uint16_t track) { sendCommand(MID_PLAYFROMMP3, track); }

    // Insert an "advertisement."
    //
    // This interrupts a track to play a track from a folder named
    // "ADVERT".  The track must have a file name as described in
    // the playTrack(uint16_t) command above.  When the advert track
    // has completed, the interruped audio track resumes from where
    // it was.
    //
    // This is typically used with the regular audio in the "MP3"
    // folder described above, but it can interrupt any track
    // regardless of how you started playing it.
    //
    // If no track is currently playing (e.g., if the device is
    // stopped or paused), this will result in an "insertion error."
    //
    // You cannot insert while an inserted track is alrady playing.
    void insertAdvert(uint16_t track) { sendCommand(MID_INSERTADVERT, track); }

    // Stops a track that was inserted with `insertAdvert`.  The
    // interrupted track will resume from where it was.
    void stopAdvert() { sendCommand(MID_STOPADVERT); }

    // Stops any audio that's playing and resets the playback
    // sequence to `SEQ_SINGLE`.
    void stop() { sendCommand(MID_STOP); }

    // Pauses the current playback.
    void pause() { sendCommand(MID_PAUSE); }

    // Undoes a previous call to `pause`.
    //
    // Alternative use:  When a track finishes playing while the
    // playback sequence is `SEQ_SINGLE`, the next track (by file index)
    // is cued up and paused.  If you call this function about 100 ms
    // after an `onTrackFinished` notification, the cued track will
    // begin playing.
    void unpause() { sendCommand(MID_UNPAUSE); }

    // Set the volume to a level in the range of 0 - 30.
    void setVolume(int volume) {
      // Catalex effectively goes to 31, but it doesn't automatically
      // clamp values.  DF Player Mini goes to 30 and clamps there.
      // We'll make them behave the same way.
      if (volume < 0) volume = 0;
      if (30 < volume) volume = 30;
      sendCommand(MID_SETVOLUME, static_cast<uint16_t>(volume));
    }

    // Selecting an equalizer interrupts the current playback, so it's
    // best to select the EQ before starting playback.  Alternatively,
    // you can also pause, select the new EQ, and then unpause.
    void selectEQ(Equalizer eq) { sendCommand(MID_SELECTEQ, eq); }

    // Sleeping doesn't seem useful.  To lower the current draw, use
    // `disableDAC`.
    void sleep() { sendCommand(MID_SLEEP); }
    
    // Seems buggy.  Try reset() or selectSource().
    void wake() { sendCommand(MID_WAKE); }

    // Disabling the DACs when not in use saves a few milliamps.
    // Causes a click on the output.
    void disableDACs() { sendCommand(MID_DISABLEDAC, 1); }

    // To re-enable the DACs after they've been disabled.
    // Causes a click on the output.
    void enableDACs() { sendCommand(MID_DISABLEDAC, 0); }

    // Ask how many audio files (total) are on a source device, including
    // the root directory and any subfolders.  This is useful for knowing
    // the upper bound on a `playFile` call.  Hook `onDeviceFileCount`
    // for the result.
    void queryFileCount(Device device) {
      switch (device) {
        case DEV_USB:    sendQuery(MID_USBFILECOUNT);   break;
        case DEV_SDCARD: sendQuery(MID_SDFILECOUNT);    break;
        case DEV_FLASH:  sendQuery(MID_FLASHFILECOUNT); break;
        default: break;
      }
    }

    void queryCurrentFile(Device device) {
      switch (device) {
        case DEV_USB:    sendQuery(MID_CURRENTUSBFILE);   break;
        case DEV_SDCARD: sendQuery(MID_CURRENTSDFILE);    break;
        case DEV_FLASH:  sendQuery(MID_CURRENTFLASHFILE); break;
        default: break;
      }
    }

    // Ask how many folders there are under the root folder on the current
    // source device.
    void queryFolderCount() { sendQuery(MID_FOLDERCOUNT); }

    // Ask which device is currently selected as the source and whether
    // it's playing, paused, or stopped.  Can also indicate if the module
    // is asleep.  Hook `onStatus` for the result.  (Current device doesn't
    // seem to be reliable on DFPlayer Mini.)
    void queryStatus() { sendQuery(MID_STATUS); }

    // Query the current volume.
    //
    // Hook `onVolume` for the result.
    void queryVolume() { sendQuery(MID_VOLUME); }

    // Query the current equalizer setting.
    //
    // Hook `onEqualizer` for the result.
    void queryEQ() { sendQuery(MID_EQ); }

    // Query the current playback sequence.
    //
    // Hook `onPlaybackSequence` for the result.
    void queryPlaybackSequence() { sendQuery(MID_PLAYBACKSEQUENCE); }

    // Query the firmware version.
    //
    // Hook `onFirmwareVersion` for the result.  Catalex doesn't respond
    // to this query, so watch for a timeout error.
    void queryFirmwareVersion() { sendQuery(MID_FIRMWAREVERSION); }

  private:
    void checkForIncomingMessage() {
      // To avoid the possibility of getting stuck in this loop
      // indefinitely, get a snapshot of the number of bytes
      // available now and process just those.  If processing
      // causes another message to arrive immediately, it'll be
      // handled on the next call.
      for (auto i = m_stream.available(); i > 0; --i) {
        if (m_in.receive(m_stream.read())) {
          receiveMessage(m_in);
        }
      }
    }

    void checkForTimeout() {
      if (m_timeout.expired()) {
        m_timeout.cancel();
        if (m_handler) m_handler->onTimedOut();
      }
    }

    void receiveMessage(const Message &msg) {
      m_timeout.cancel();
      if (m_handler) m_handler->onMessageReceived(msg);
    }

    void sendMessage(const Message &msg) {
      const auto buf = msg.getBuffer();
      const auto len = msg.getLength();
      m_stream.write(buf, len);
      m_timeout.set(200);
      if (m_handler) m_handler->onMessageSent(msg);
    }

    void sendCommand(MsgID msgid, uint16_t param = 0, bool feedback = true) {
      m_out.set(msgid, param, feedback ? Message::FEEDBACK : Message::NO_FEEDBACK);
      sendMessage(m_out);
    }

    void sendQuery(MsgID msgid, uint16_t param = 0) {
      // Since queries naturally have a response, we won't ask for feedback.
      sendCommand(msgid, param, false);
    }

    Stream  &m_stream;
    Message  m_in;
    Message  m_out;
    Timeout<MillisClock> m_timeout;
    Device   m_source;   // the currently selected device
    uint16_t m_files;    // the number of files on the selected device
    uint8_t  m_folders;  // the number of folders on the selected device
    BasicAudioEventHandler *m_handler;
};

template <typename SerialType>
class AudioModule : public BasicAudioModule {
  public:
    explicit AudioModule(SerialType &serial) :
      BasicAudioModule(serial), m_serial(serial) {}

    // Initialization to be done during `setup`.
    void begin(BasicAudioEventHandler *handler) override {
      m_serial.begin(9600);
      BasicAudioModule::begin(handler);
    }

  private:
    SerialType &m_serial;
};

template <typename SerialType>
AudioModule<SerialType> make_AudioModule(SerialType &serial) {
  return AudioModule<SerialType>(serial);
}

class AdvancedAudioEventHandler : public BasicAudioEventHandler {
  public:
    void onMessageReceived(const Message &msg) override;
    void onTimedOut() override;

    virtual void onAck() {};
    virtual void onCurrentTrack(Device /*src*/, uint16_t /*track*/) {};
    virtual void onDeviceInserted(Device /*src*/) {};
    virtual void onDeviceFileCount(Device /*src*/, uint16_t /*count*/) {};
    virtual void onDeviceRemoved(Device /*src*/) {};
    virtual void onError(ErrorCode /*code*/) {};
    virtual void onEqualizer(Equalizer /*eq*/) {};
    virtual void onFinishedFile(Device /*src*/, uint16_t /*file_index*/) {};
    virtual void onFirmwareVersion(uint16_t /*version*/) {};
    virtual void onFolderCount(uint16_t /*count*/) {};
    virtual void onFolderTrackCount(uint16_t /*count*/) {};
    virtual void onInitComplete(uint16_t /*devices*/) {};
    virtual void onMessageInvalid() {};
    virtual void onPlaybackSequence(Sequence /*seq*/) {};
    virtual void onStatus(Device /*device*/, ModuleState /*state*/) {};
    virtual void onVolume(uint8_t /*volume*/) {};
};

#ifndef NDEBUG
class DebugAudioEventHandler : public AdvancedAudioEventHandler {
  public:
    void onMessageSent(const Message &msg) override;
    void onMessageReceived(const Message &msg) override;
    void onTimedOut() override;

    void onAck() override;
    void onCurrentTrack(Device src, uint16_t track) override;
    void onDeviceInserted(Device src) override;
    void onDeviceFileCount(Device src, uint16_t count) override;
    void onDeviceRemoved(Device src) override;
    void onError(ErrorCode code) override;
    void onEqualizer(Equalizer eq) override;
    void onFinishedFile(Device device, uint16_t file_index) override;
    void onFirmwareVersion(uint16_t version) override;
    void onFolderCount(uint16_t count) override;
    void onFolderTrackCount(uint16_t count) override;
    void onInitComplete(uint16_t devices) override;
    void onMessageInvalid() override;
    void onPlaybackSequence(Sequence seq) override;
    void onStatus(Device device, ModuleState state) override;
    void onVolume(uint8_t volume) override;

  protected:
    static void printDeviceName(Device src);
    static void printEqualizerName(Equalizer eq);
    static void printModuleStateName(ModuleState state);
    static void printSequenceName(Sequence seq);
    static void printMessageBytes(const Message &msg);
};
#endif


#endif
