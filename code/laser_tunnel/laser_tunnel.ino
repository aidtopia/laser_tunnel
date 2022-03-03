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
  Calibrating,
  Idle,         // waiting for a trigger
  Running,      // effect animation in progress
  Stopped       // we're in the emergency stop
} state = State::Initializing;

// Issuing fog is also not part of the state machine, but part
// of the animation sequence.

// Each bit in the pattern determines when the laser should
// switch on or off.  Scanning the pattern begins with each
// falling edge in the fan's tachometer signal, which occurs
// twice per revolution.  Scanning wraps if the entire pattern
// is used before the next tachometer pulse.
const uint8_t pattern[] = {
  0b10101010,
  0b11001100,
  0b11001100,
  0b11110000,
  0b11110000,
  0b11110000,
  0b11110000,
  0b11111111,
  0b00000000,
  0b11111111,
  0b00000000,
  0b11111111,
  0b00000000,
  0b11111111,
  0b00000000,
  0b11111111,
  0b11111111,
  0b00000000,
  0b00000000,
  0b11111111,
  0b11111111,
  0b00000000,
  0b00000000,
  0b11111111,
  0b11111111,
  0b00000000,
  0b00000000,
  0b11111111,
  0b11111111,
  0b00000000,
  0b00000000,
  0b00000000
};

constexpr uint16_t pattern_byte_count = sizeof(pattern);
constexpr uint16_t pattern_size = 8*pattern_byte_count;

// `scan_start` is a bit offset into the pattern where the
// scan should begin when the tachometer pulse is detected.
// By animating `scan_start`, the tunnel appears to revolve.
volatile uint8_t scan_byte = 0;
volatile uint8_t scan_mask = 0b10000000;
volatile uint16_t scan_start = 0;

// Since there are two pulses per revolution, we need to ignore
// every other pulse.  `half_rev` is a toggle used by the fan
// pulse interrupt service routines to skip every other pulse.
// The variable could be function static, but that generates
// slower code.
bool half_rev = false;

// When running the effect, we make sure to align the pixel
// clock and the pattern scanning to the beginning of each
// revolution.
void fanPulseISR() {
  half_rev = !half_rev;
  if (half_rev) return;
  pixel_clock.resync();  // keep the pixel clock aligned with revolutions
  scan_byte = scan_start >> 3;
  scan_mask = 0b10000000u >> (scan_start & 0b0111);
}

// This is the pixel clock ISR.
ISR(TIMER2_COMPA_vect) {
  if (scan_mask & pattern[scan_byte]) {
    laser.on();
  } else {
    laser.off();
  }
  
  if (scan_mask != 1) {
    scan_mask >>= 1;
  } else {
    scan_mask = 0b10000000;
    scan_byte = (scan_byte + 1) % pattern_byte_count;
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
  Serial.println(F("Triggered."));
  fog_pin.set();
  soundfx.play(SoundFX::STARTLE);
  state = State::Running;
}

void endEffect() {
  Serial.println(F("Effect ended."));
  fog_pin.clear();
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
#ifndef NDEBUG
        const auto freq = 1000000ul * 100ul / period;
        const auto rpm = (60 * freq + 50) / 100;
        Serial.print(F("  Revolution time: "));
        Serial.print(period);
        Serial.println(F(" us (average)"));
        Serial.print(F("  Speed:           "));
        Serial.print(rpm);
        Serial.println(F(" RPM"));
#endif
        
        const float pixel_freq = 1.0e6 * pattern_size / period;
#ifndef NDEBUG
        Serial.print(F("For "));
        Serial.print(pattern_size);
        Serial.print(F(" pixels per revolution, PixelTimer must run at "));
        Serial.print(pixel_freq);
        Serial.println(F(" Hz."));
#endif
        pixel_clock.begin(pixel_freq);

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
      break;
    case State::Running:
      delay(16);
      noInterrupts();
      scan_start = (scan_start + 1) % (pattern_size);
      interrupts();
      if (!soundfx.isBusy()) {
        if (trigger_high.read() == HIGH || trigger_low.read() == LOW) {
          // Don't bother going idle, just run another round.
          beginEffect();
        } else {
          endEffect();
        }
      }
      break;
    default:
      Serial.print(F("Laser Tunnel in unexpected state: "));
      Serial.println(static_cast<int>(state));
      emergencyStop();
      break;
  }
}
