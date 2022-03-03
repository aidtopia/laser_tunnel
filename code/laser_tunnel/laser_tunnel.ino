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
#include "fan.h"
#include "laser.h"
#include "pins.h"
#include "soundfx.h"
#include "timeout.h"
#include "timers.h"

// MCU Resources
auto fan                  = Fan(/*tach=*/2, /*pwm=*/3);
auto laser                = Laser(4);
const auto emergency_stop = DigitalInputPin(5);
const auto suppress_high  = DigitalInputPin(6);
const auto suppress_low   = DigitalInputPin(7);
const auto trigger_high   = DigitalInputPin(8);
const auto trigger_low    = DigitalInputPin(9);
auto soundfx              = SoundFX(10, 12, 11);
const auto status_pin     = DigitalOutputPin(LED_BUILTIN);
const auto fog_pin        = DigitalOutputPin(14);  // a.k.a. A0
const auto house_lights_pin = DigitalOutputPin(15);  // a.k.a. A1
const auto effect_time_pin = A3;
const auto suppress_time_pin = A2;

Timer<2> pixel_clock;

enum class State {
  Initializing,
  Calibrating,
  Idle,         // waiting for a trigger
  Running,      // effect animation in progress
  Stopped       // we're in the emergency stop
} state = State::Initializing;

// Suppress is not part of the state machine.  When supressed,
// the laser is disabled until the suppress signal has been LOW
// for the suppress time.
Timeout<MillisClock> suppress;

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
volatile unsigned long rev_time = 0;

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

// During calibration, this ISR notes the time (in microseconds)
// for each revolution.
void fanCalibrationISR() {
  half_rev = !half_rev;
  if (half_rev) return;
  rev_time = micros();
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

unsigned long measureFanPeriod() {
  Serial.print("Measuring fan speed");
  auto avg_period = 0ul;
  auto last_rev_time = micros();
  fan.run(fanCalibrationISR);
  for (auto i = 0; i < 5; ++i) {
    Serial.print('.');
    Serial.flush();
    for (auto samples = 50; samples > 0; ) {
      if (emergency_stop.read() == LOW) emergencyStop();
      if (rev_time) {
        noInterrupts();
        const auto this_rev_time = rev_time;
        rev_time = 0;
        interrupts();
        const auto period = this_rev_time - last_rev_time;
        last_rev_time = this_rev_time;
        avg_period = (15*avg_period + period + 8) / 16;
        --samples;
        status_pin.toggle();
      }
    }
  }
  fan.stop();
  status_pin.clear();
  Serial.println();

  const auto freq = 1000000ul * 100ul / avg_period;
  const auto rpm = (60 * freq + 50) / 100;
  Serial.print(F("  Revolution time: "));
  Serial.print(avg_period);
  Serial.println(F(" us (average)"));
  Serial.print(F("  Speed:           "));
  Serial.print(rpm);
  Serial.println(F(" RPM"));

  return avg_period;
}

[[noreturn]] void emergencyStop() {
  noInterrupts();
  laser.disable();
  fan.stop();
  pixel_clock.stop();
  interrupts();
  soundfx.stop();
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
  soundfx.playStartleSound();
  state = State::Running;
}

void endEffect() {
  Serial.println(F("Effect ended."));
  fog_pin.clear();
  state = State::Idle;
}

void suppressLaser() {
  if (!suppress.active()) {
    Serial.println(F("Suppressing!"));
    laser.disable();

    // temp for debugging
    const auto setting = analogRead(suppress_time_pin);
    const auto duration =
      map(setting, 1023, 0, 1000, 60000);
    Serial.print(F("Suppress Time: "));
    Serial.print(setting);
    Serial.print(F(" = "));
    Serial.print(duration);
    Serial.println(F(" ms"));
  }
  // We keep updating the suppress time as long as a suppress
  // sensor is activated.  Effectively, the suppress time applies
  // once the sensor is deactivated.
  const auto duration =
    map(analogRead(suppress_time_pin), 1023, 0, 3000, 30000);
  suppress.set(duration);
}

void unsuppressLaser() {
  if (suppress.active()) {
    Serial.println(F("Suppress time has elapsed."));
    laser.enable();
    suppress.cancel();
  }
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
  suppress_high.begin();
  suppress_low.begin(INPUT_PULLUP);
  trigger_high.begin();
  trigger_low.begin(INPUT_PULLUP);

  const auto fan_period_us = measureFanPeriod();
  const float pixel_freq = 1.0e6 * pattern_size / fan_period_us;
  Serial.print("For ");
  Serial.print(pattern_size);
  Serial.print(" pixels per revolution, PixelTimer must run at ");
  Serial.print(pixel_freq);
  Serial.println(" Hz.");

  pixel_clock.begin(pixel_freq);
  fan.run(fanPulseISR);
  Serial.println("Initialization complete.");
  state = State::Idle;
}

void loop() {
  if (emergency_stop.read() == LOW) emergencyStop();

  if (suppress_high.read() == HIGH || suppress_low.read() == LOW) {
    suppressLaser();
  } else if (suppress.expired()) {
    unsuppressLaser();
  }

  soundfx.update();
  
  switch (state) {
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
        }
        endEffect();
      }
      break;
    default:
      Serial.print(F("Laser Tunnel in unexpected state: "));
      Serial.println(static_cast<int>(state));
      emergencyStop();
      break;
  }
}
