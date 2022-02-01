// Laser Tunnel
// Adrian McCarthy 2022

// Controls a PC fan and a laser to create a tunnel-like cone of light.
// Designed for a 4-pin fan (GND, 12VDC, Tachometer, PWM control) and a
// 5-volt 5 mW laser module.

// This code was developed on a 5-volt Arduino Pro Mini, but it should
// be portable to any 5V Arduino board built around an AVR processor
// that runs at least 16 MHz.

#include <Arduino.h>
#include "pins.h"
#include "pixel_clock.h"

constexpr auto fan_tach_pin = 2;
static_assert(digitalPinToInterrupt(fan_tach_pin) != NOT_AN_INTERRUPT,
              "The tachometer output from the fan must be connected "
              "to a pin that can generate external interrupts.");

const auto fan_pwm_pin    = DigitalOutputPin(3);
const auto laser_pwm_pin  = DigitalOutputPin(4);
const auto emergency_stop = DigitalInputPin(5);
const auto status_pin     = DigitalOutputPin(LED_BUILTIN);

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

// `scan_start` is a bit offset into the pattern where the
// scan should begin when the tachometer pulse is detected.
// By animating `scan_start`, the tunnel appears to revolve.
volatile uint8_t scan_byte = 0;
volatile uint8_t scan_mask = 0b10000000;
volatile uint16_t scan_start = 0;
volatile bool pulse_flag = false;

void fanPulseISR() {
  pulse_flag = !pulse_flag;
  if (pulse_flag) return;
  PixelClock::resync();  // keep the pixel clock aligned with revolutions
  scan_byte = scan_start >> 3;
  scan_mask = 0b10000000u >> (scan_start & 0b0111);
}

ISR(TIMER2_COMPA_vect) {
  if (scan_mask & pattern[scan_byte]) laser_pwm_pin.set(); else laser_pwm_pin.clear();
  
  if (scan_mask != 1) {
    scan_mask >>= 1;
  } else {
    scan_mask = 0b10000000;
    scan_byte = (scan_byte + 1) % sizeof(pattern);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("Laser Tunnel V1"));
  Serial.println(F("Adrian McCarthy (a.k.a., Hayward Haunter) 2022"));

  status_pin.begin(LOW);
  laser_pwm_pin.begin(LOW);

  pinMode(fan_tach_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(fan_tach_pin), fanPulseISR, FALLING);

  fan_pwm_pin.begin(HIGH);

  emergency_stop.begin(INPUT_PULLUP);

  PixelClock::begin();
  PixelClock::compute(8*sizeof(pattern), 1800);
}

void loop() {
  if (emergency_stop.read() == LOW) emergencyStop();
  delay(16);
  noInterrupts();
  scan_start = (scan_start + 1) % (8*sizeof(pattern));
  interrupts();
}

[[noreturn]] void emergencyStop() {
  noInterrupts();
  laser_pwm_pin.clear();
  fan_pwm_pin.clear();
  PixelClock::stop();  // to ensure laser isn't switched back on
  interrupts();
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
