// Laser Tunnel
// Adrian McCarthy 2022

// Controls a PC fan and a laser to create a tunnel-like cone of light.
// Designed for a 4-pin fan (GND, 12VDC, Tachometer, PWM control) and a
// 5-volt 5 mW laser module.

// This code was developed on a 5-volt Arduino Pro Mini, but it should
// be portable to any 5V Arduino board built around an AVR processor
// that runs at least 16 MHz.

class DigitalPin {
  public:
    explicit DigitalPin(int8_t p) : m_pin(p) {}

    operator int() const { return m_pin; }

  protected:
    void mode(int mode) const { pinMode(m_pin, mode); }
    uint8_t mask() const { return digitalPinToBitMask(m_pin); }
    uint8_t volatile * outputReg() const { return portOutputRegister(digitalPinToPort(m_pin)); }
    uint8_t const volatile * inputReg() const { return portInputRegister(digitalPinToPort(m_pin)); }

  private:
    int8_t m_pin;
};

class DigitalInputPin : public DigitalPin {
  public:
    explicit DigitalInputPin(int8_t p) :
      DigitalPin(p),
      m_mask(mask()),
      m_input(inputReg()) {}

    void begin() const { mode(INPUT); }
    void begin(int x) const {
      mode(x == INPUT_PULLUP ? INPUT_PULLUP : INPUT);
    }
    
    int read() const { return (*m_input & m_mask) ? HIGH : LOW; }

  private:
      uint8_t m_mask;
      uint8_t const volatile *m_input;
};

class DigitalOutputPin : public DigitalPin {
  public:
    explicit DigitalOutputPin(int8_t p) :
      DigitalPin(p),
      m_mask(mask()),
      m_output(outputReg()) {}
  
      void begin(int initial=LOW) const {
        mode(OUTPUT);
        write(initial);
      }
  
      void clear()  const { *m_output &= ~m_mask; }
      void set()    const { *m_output |=  m_mask; }
      void toggle() const { *m_output ^=  m_mask; }
      void write(int value) const {
        if (value == LOW) clear(); else set();
      }

  private:
      uint8_t m_mask;
      uint8_t volatile *m_output;
};

constexpr auto fan_tach_pin = 2;
static_assert(digitalPinToInterrupt(fan_tach_pin) != NOT_AN_INTERRUPT,
              "The tachometer output from the fan must be connected "
              "to a pin that can generate external interrupts.");

const auto fan_pwm_pin   = DigitalOutputPin(3);
const auto laser_pwm_pin = DigitalOutputPin(4);
const auto emergency_stop = DigitalInputPin(5);
const auto status_pin    = DigitalOutputPin(LED_BUILTIN);

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
  0b00000000
};

// `scan_start` is a bit offset into the pattern where the
// scan should begin when the tachometer pulse is detected.
// By animating `scan_start`, the tunnel appears to revolve.
volatile uint8_t scan_byte = 0;
volatile uint8_t scan_mask = 0b10000000;
volatile uint16_t scan_start = 0;
volatile bool pulse_flag = false;

void startPixelTimer() {
  // Set the wave generation mode (3 bits across two registers) to
  // compare timer/counter (CTC) to OCR2A.
  TCCR2A = (TCCR2A & 0b11111100) | (1 << WGM21);
  TCCR2B = (TCCR2B & 0b11110111);
  // Set the clock source prescaler to 64.
  TCCR2B = (TCCR2B & 0b11111000) | (1 << CS22);
  TCNT2 = 0;  // start counting from 0.
  OCR2A = 16;  // how high to count
  TIMSK2 |= (1 << OCIE2A);  // enable interrupt each time the counter reaches the limit
}

void stopPixelTimer() {
  // Change the clock source to none.
  TCCR2B = 0;
  TIMSK2 = 0;
}

// Sets the pixel timer back to zero.
void resyncPixelTimer() {
  TCNT2 = 0;
}

void fanPulseISR() {
  pulse_flag = !pulse_flag;
  if (pulse_flag) return;
  resyncPixelTimer();  // keep the pixel clock aligned with revolutions
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
  Serial.println("Laser Tunnel by Hayward Haunter");

  status_pin.begin(LOW);
  laser_pwm_pin.begin(LOW);

  pinMode(fan_tach_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(fan_tach_pin), fanPulseISR, FALLING);

  fan_pwm_pin.begin(HIGH);

  emergency_stop.begin(INPUT_PULLUP);

  // Use Timer/Counter2 to generate per-pixel interrupts.
  startPixelTimer();
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
  stopPixelTimer();  // to ensure laser isn't switched back on
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
