// Laser Tunnel
// Adrian McCarthy 2022

// Controls a PC fan and a laser to create a tunnel-like cone of light.
// Designed for a 4-pin fan (GND, 12VDC, Tachometer, PWM control) and a
// 5 mW laser module with a PWM pin.

constexpr auto fan_tach_pin = 2;  // Must be a pin that generates external interrupts
constexpr auto fan_pwm_pin  = 3;
constexpr auto laser_pwm_pin = 6;

// Each bit in the pattern determines when the laser should
// switch on or off.  Scanning the pattern begins with each
// falling edge in the fan's tachometer signal, which occurs
// twice per revolution.  Scanning wraps if the entire pattern
// is used before the next techometer pulse.
const uint32_t pattern = 0b11111111111100000000000100000000UL;
// The bit in `scan_start` determines where in the pattern
// scanning begins.
volatile uint32_t scan_start = 1UL << 31;
// The bit in the `scan_pos` indicates the current position in
// the scan.
volatile uint32_t scan_pos = scan_start;

void fanPulseISR() {
  // Restart Timer/Counter2 and the `scan_pos`.  This will keep
  // the pattern aligned with the tachometer pulses.
  TCNT2 = 0;
  scan_pos = scan_start;

  // By animating `scan_start`, the tunnel appears to revolve.
  scan_start <<= 1;
  if (scan_start == 0) scan_start = 1;
}

ISR(TIMER2_COMPA_vect) {
  static const auto port = digitalPinToPort(laser_pwm_pin);
  static const auto mask = digitalPinToBitMask(laser_pwm_pin);
  static uint8_t volatile * const reg = portOutputRegister(port);
  if (scan_pos & pattern) {
    *reg |= mask;
  } else {
    *reg &= ~mask;
  }
  scan_pos >>= 1;
  if (scan_pos == 0) scan_pos = (1UL << 31);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Laser Tunnel by Hayward Haunter");

  pinMode(laser_pwm_pin, OUTPUT);
  digitalWrite(laser_pwm_pin, LOW);

  pinMode(fan_tach_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(fan_tach_pin), fanPulseISR, FALLING);

  pinMode(fan_pwm_pin, OUTPUT);
  digitalWrite(fan_pwm_pin, HIGH);

  // Use Timer/Counter2 to generate interrupts.  In the ISR, we'll
  // whether the laser should be on or off.  We'll restart the timer
  // each time we get a pulse from the tachometer (twice per
  // revolution), so our pattern of laser dots should be stable.
  noInterrupts();
  // Set the wave generation mode (3 bits across two registers) to CTC to OCR2A.
  TCCR2A = (TCCR2A & 0b11111100) | (1 << WGM21);
  TCCR2B = (TCCR2B & 0b11110111);
  // Set the clock source prescaler to 64.
  TCCR2B = (TCCR2B & 0b11111000) | (1 << CS22);
  TCNT2 = 0;  // start counting from 0.
  OCR2A = 16;  // how high to count
  TIMSK2 |= (1 << OCIE2A);  // enable interrupt each time the counter reaches the limit
  interrupts();
}

void loop() {
  delay(250);
}
