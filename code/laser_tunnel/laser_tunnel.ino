// Laser Tunnel
// Adrian McCarthy 2022

// Controls a PC fan and a laser to create a tunnel-like cone of light.
// Designed for a 4-pin fan (GND, 12VDC, Tachometer, PWM control) and a
// 5-volt 5 mW laser module.

// This code was developed on a 5-volt Arduino Pro Mini, but it should
// be portable to any 5V/16 MHz AVR processor.

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "aidassert.h"
#include "calibrator.h"
#include "fan.h"
#include "laser.h"
#include "patternbuffer.h"
#include "pins.h"
#include "soundfx.h"
#include "suppressor.h"
#include "timeout.h"
#include "timers.h"

// MCU Resources
auto fan                  = Fan(/*tach=*/2, /*pwm=*/3);
auto laser                = Laser(4);
const auto emergency_stop = DigitalInputPin(5);
auto suppressor           = Suppressor(6, 7, A2);
const auto trigger_high   = DigitalInputPin(8);
const auto trigger_low    = DigitalInputPin(9);
auto soundfx              = SoundFX(10, 12, 11);
const auto status_pin     = DigitalOutputPin(LED_BUILTIN);
const auto fog_pin        = DigitalOutputPin(14);  // a.k.a. A0
const auto house_lights_pin = DigitalOutputPin(15);  // a.k.a. A1
const auto effect_time_pin = A3;

Calibrator calibrator;
Timer<2> pixel_clock;

enum class State {
  Initializing,
  Calibrating,  // measuring fan speed to set pixel clock
  Idle,         // waiting for a trigger
  Animating,
  Stopped       // we're in the emergency stop
} state = State::Initializing;

Timeout<MillisClock> effect_timeout;
Timeout<MillisClock> fog_timeout;

PatternBuffer pattern;
volatile uint8_t scan_start = 0;
volatile uint8_t scan_index = 0;

// Since there are two pulses per revolution, we need to ignore
// every other pulse.  `half_rev` is a toggle used by the fan
// pulse interrupt service routine to skip every other pulse.
// The variable could be function static, but that generates
// slower code.
bool half_rev = false;

// Re-align the pixel clock and the pattern scanning to the
// beginning of each revolution.
void fanPulseISR() {
  half_rev = !half_rev;
  if (half_rev) return;
  pixel_clock.resync();  // keep the pixel clock aligned with revolutions
  scan_index = scan_start;
}

// This is the pixel clock ISR.
ISR(TIMER2_COMPA_vect) {
  if (pattern[scan_index++]) {
    laser.on();
  } else {
    laser.off();
  }
}

[[noreturn]] void emergencyStop() {
  noInterrupts();
  laser.disable();
  fan.stop();
  pixel_clock.stop();
  interrupts();
  soundfx.play(SoundFX::EMERGENCY);
  fog_pin.clear();
  house_lights_pin.set();

  // We're never going to return, so this has no effect, but
  // we'll set it for consistency.
  state = State::Stopped;

  Serial.println("Emergency Stop!");
  Serial.println("Reset the microcontroller to restart.");
  for (;;) {
    constexpr auto dot = 200;  // milliseconds
    constexpr auto dash = 3*dot;
    for (const char ch : "...---... ") {
      switch (ch) {
        case '.': status_pin.set(); delay(dot); status_pin.clear(); delay(dot); break;
        case '-': status_pin.set(); delay(dash); status_pin.clear(); delay(dot); break;
        case ' ': delay(dash); break;
        default: break;
      }
    }
  }
}

void beginEffect() {
  pattern.setTestPattern();  // temporary

  const auto audio_duration = soundfx.duration(SoundFX::STARTLE);
  if (audio_duration != 0) {
    // We know how long to run the effect to match the audio track,
    // so the Effect Time pot tells us the duty cycle for the fog.
    const auto fog_duty =
      map(analogRead(effect_time_pin), 1023, 0, 0, 100);
    fog_pin.set();
    fog_timeout.set(fog_duty * audio_duration / 100);
    soundfx.play(SoundFX::STARTLE);
    state = State::Animating;
    return;
  }

  if (soundfx.has(SoundFX::STARTLE)) {
    // The effect will run for the duration of the audio track,
    // but we don't know how long that will be.  We'll run the
    // fog for the duration indicated by the Effect Time pot.
    // (If the audio completes sooner, we'll stop the fog then.)
    fog_pin.set();
    const auto fog_duration =
      map(analogRead(effect_time_pin), 1023, 0, 3, 30)*1000;
    fog_timeout.set(fog_duration);
    soundfx.play(SoundFX::STARTLE);
    state = State::Animating;
    return;
  }

  // There's no audio, so the Effect Time pot tells us how
  // long to animate, and we'll blast fog for the first half
  // of that (up to 1 minute).
  const auto effect_duration =
    map(analogRead(effect_time_pin), 1023, 0, 3, 30)*1000;
  effect_timeout.set(effect_duration);
  fog_pin.set();
  const auto fog_duration = min(effect_duration/2, 60000);
  fog_timeout.set(fog_duration);
  state = State::Animating;
}

void endEffect() {
  fog_pin.clear();
  fog_timeout.cancel();
  effect_timeout.cancel();
  pattern.clear();
  soundfx.play(SoundFX::AMBIENT);
  state = State::Idle;
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("\nLaser Tunnel V1"));
  Serial.println(F("Copyright 2022 Adrian McCarthy"));
  Serial.println(F("https://github.com/aidtopia/laser_tunnel"));

  status_pin.begin(LOW);
  laser.begin();
  fan.begin();
  soundfx.begin();
  fog_pin.begin(LOW);
  house_lights_pin.begin(LOW);

  emergency_stop.begin(INPUT_PULLUP);
  suppressor.begin();
  trigger_high.begin();
  trigger_low.begin(INPUT_PULLUP);

  state = State::Calibrating;
  calibrator.begin(fan);
}

void loop() {
  if (emergency_stop.read() == LOW) emergencyStop();
  suppressor.update(laser);
  soundfx.update();
  
  switch (state) {
    case State::Calibrating:
      if (calibrator.update()) {
        const auto period = calibrator.fanPeriod();
        const auto pixel_freq =
          calibrator.pixelFrequency(period, pattern.size());
        pixel_clock.begin(pixel_freq);

        // Once the pixel clock is started, we can run the fan
        // with its usual ISR.
        fan.run(fanPulseISR);

        // By now, the soundfx module should be ready.
        soundfx.play(SoundFX::AMBIENT);

        state = State::Idle;
      }
      break;

    case State::Idle:
      if (trigger_high.read() == HIGH || trigger_low.read() == LOW) {
        beginEffect();
      }
      if (soundfx.currentTrack() == SoundFX::NONE && soundfx.has(SoundFX::AMBIENT)) {
        soundfx.play(SoundFX::AMBIENT);
      }
      break;

    case State::Animating:
      delay(16);  // TODO:  Eliminate and compute the animation based
                  // on actual millis() time.
      noInterrupts();
      scan_start -= 1;
      interrupts();
      pattern.togglePixel(random(pattern.size()));

      if (fog_timeout.expired()) {
        fog_pin.clear();
        fog_timeout.cancel();
      }

      if (soundfx.currentTrack() == SoundFX::STARTLE) break;

      if (effect_timeout.active() && !effect_timeout.expired()) break;
      
      if (trigger_high.read() == HIGH || trigger_low.read() == LOW) {
        // Don't bother going idle, just run another round.
        beginEffect();
        break;
      }
      
      endEffect();
      break;

    default:
      Serial.print(F("Laser Tunnel in unexpected state: "));
      Serial.println(static_cast<int>(state));
      emergencyStop();
      break;
  }
}
