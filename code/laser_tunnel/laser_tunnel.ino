// Laser Tunnel
// Adrian McCarthy 2022

// Controls a PC fan and a laser to create a tunnel-like cone of light.
// Designed for a 4-pin fan (GND, 12VDC, Tachometer, PWM control) and a
// 5-volt 5 mW laser module.

// This code was developed on a 5-volt Arduino Pro Mini, but it should
// be portable to any 5V/16 MHz AVR processor.

#include <Arduino.h>
#include "aidassert.h"
#include "pins.h"
#include "timers.h"

// MCU Resources
const auto fan_tach_pin   = DigitalInputPin(2);
const auto fan_pwm_pin    = DigitalOutputPin(3);
const auto laser_pwm_pin  = DigitalOutputPin(4);
const auto emergency_stop = DigitalInputPin(5);
const auto suppress_high  = DigitalInputPin(6);
const auto suppress_low   = DigitalInputPin(7);
const auto trigger_high   = DigitalInputPin(8);
const auto trigger_low    = DigitalInputPin(9);
const auto status_pin     = DigitalOutputPin(LED_BUILTIN);
const auto fog_pin        = DigitalOutputPin(14);  // a.k.a. A0
const auto house_lights_pin = DigitalOutputPin(15);  // a.k.a. A1

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

constexpr uint16_t pattern_size = 8*sizeof(pattern);

// `scan_start` is a bit offset into the pattern where the
// scan should begin when the tachometer pulse is detected.
// By animating `scan_start`, the tunnel appears to revolve.
volatile uint8_t scan_byte = 0;
volatile uint8_t scan_mask = 0b10000000;
volatile uint16_t scan_start = 0;
volatile unsigned long pulse_time = 0;

// `pulse_flag` could be function static, but that generates
// slower code.
bool pulse_flag = false;
void fanPulseISR() {
  pulse_flag = !pulse_flag;
  if (pulse_flag) return;
  pixel_clock.resync();  // keep the pixel clock aligned with revolutions
  scan_byte = scan_start >> 3;
  scan_mask = 0b10000000u >> (scan_start & 0b0111);
  pulse_time = micros();
}

ISR(TIMER2_COMPA_vect) {
  if (scan_mask & pattern[scan_byte]) {
    laser_pwm_pin.set();
  } else {
    laser_pwm_pin.clear();
  }
  
  if (scan_mask != 1) {
    scan_mask >>= 1;
  } else {
    scan_mask = 0b10000000;
    scan_byte = (scan_byte + 1) % sizeof(pattern);
  }
}

[[noreturn]] void emergencyStop() {
  noInterrupts();
  laser_pwm_pin.clear();
  fan_pwm_pin.clear();
  pixel_clock.stop();  // to ensure laser isn't switched back on
  interrupts();
  fog_pin.clear();
  house_lights_pin.set();

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

unsigned long measureFanPeriod() {
  Serial.println("Measuring fan speed...");
  auto avg_period = 0ul;

  auto last_pulse_time = micros();
  fan_pwm_pin.set();
  for (auto samples = 250; samples > 0; ) {
    if (emergency_stop.read() == LOW) emergencyStop();
    if (pulse_time) {
      noInterrupts();
      const auto period = pulse_time - last_pulse_time;
      last_pulse_time = pulse_time;
      pulse_time = 0;
      interrupts();
      avg_period = (15*avg_period + period + 8) / 16;
      --samples;
      status_pin.toggle();
    }
  }
  fan_pwm_pin.clear();
  status_pin.clear();

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

void setup() {
  Serial.begin(9600);
  Serial.println(F("\nLaser Tunnel V1"));
  Serial.println(F("Copyright 2022 Adrian McCarthy"));
  Serial.println(F("https://github.com/aidtopia/laser_tunnel"));

  // The tachometer output from the fan must be connected
  // to a pin that can generate external interrupts.
  ASSERT(digitalPinToInterrupt(fan_tach_pin) != NOT_AN_INTERRUPT);

  // Initial outputs
  status_pin.begin(LOW);
  laser_pwm_pin.begin(LOW);
  fog_pin.begin(LOW);
  house_lights_pin.begin(LOW);
  fan_pwm_pin.begin(LOW);

  // Set up the inputs
  emergency_stop.begin(INPUT_PULLUP);
  pinMode(fan_tach_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(fan_tach_pin), fanPulseISR, FALLING);
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
  fan_pwm_pin.set();
  Serial.println("Initialization complete.");
  state = State::Idle;
}

void loop() {
  if (emergency_stop.read() == LOW) emergencyStop();
  switch (state) {
    case State::Idle:
      if (trigger_high.read() == HIGH || trigger_low.read() == LOW) {
        state = State::Running;
      }
      break;
    case State::Running:
      delay(16);
      noInterrupts();
      scan_start = (scan_start + 1) % (pattern_size);
      interrupts();
      break;
    default:
      Serial.print(F("Laser Tunnel in unexpected state: "));
      Serial.println(static_cast<int>(state));
      emergencyStop();
      break;
  }
}
