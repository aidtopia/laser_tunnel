// Laser Tunnel
// Adrian McCarthy 2022

// Controls a PC fan and a laser to create a tunnel-like cone of light.
// Designed for a 4-pin fan (GND, 12VDC, Tachometer, PWM control) and a
// 5 mW laser module with a PWM pin.

constexpr auto fan_tach_pin = 2;
constexpr auto fan_pwm_pin  = 3;
constexpr auto laser_pwm_pin = 6;

volatile long fan_rpm = 0;
volatile unsigned long prev_fan_pulse_time = 0;
volatile unsigned long expected_fan_pulse_time = 0;

// Interrupt service routine to update `fan_rpm` and
// `prev_fan_pulse_time`.
void fanPulseISR() {
  constexpr auto tach_pulses_per_rev = 2;
  const auto now = (micros() + 50)/100;
  const auto elapsed = now - prev_fan_pulse_time;
  const auto rpm = 600000/elapsed/tach_pulses_per_rev;
  fan_rpm = (3*fan_rpm + rpm + 2) / 4;
  prev_fan_pulse_time = now;
  expected_fan_pulse_time = now + 2*elapsed;  // estimated
}

long getFanRPM() {
  noInterrupts();
  if (micros()/100 > expected_fan_pulse_time) {
    fan_rpm /= 2;
    digitalWrite(laser_pwm_pin, LOW);
  }
  interrupts();
  return fan_rpm;
}

void setup() {
  Serial.begin(115200);
  Serial.println("targetRPM fanRPM fanPWM");

  pinMode(fan_tach_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(fan_tach_pin), fanPulseISR, FALLING);
  digitalWrite(fan_pwm_pin, LOW);  // We'll PWM it in the loop.
  pinMode(fan_pwm_pin, OUTPUT);
  pinMode(laser_pwm_pin, OUTPUT);
  digitalWrite(laser_pwm_pin, LOW);
  delay(500);  // Give the fan a chance to coast to a stop.
}

void loop() {
  static long target_rpm = 1000;
  static long i_error = 0;
  static long last_error = 0;
  const auto actual_rpm = getFanRPM();
  const auto error = target_rpm - actual_rpm;
  i_error += error;
  const auto d_error = error - last_error;
  last_error = error;
  int duty_cycle = (error + 5)/10 + (i_error + 10)/20 + (d_error + 15) / 30;
  if (duty_cycle < 0) duty_cycle = 0;
  if (255 < duty_cycle) duty_cycle = 255;
  analogWrite(fan_pwm_pin, duty_cycle);  
  Serial.print(target_rpm);
  Serial.print(' ');
  Serial.print(actual_rpm);
  Serial.print(' ');
  Serial.println(duty_cycle);

  static char buffer[64];
  static int pos = 0;
  while (Serial.available() > 0) {
    char ch = Serial.read();
    if (ch == '\n' || pos + 1 == sizeof(buffer)) {
      buffer[pos] = '\0';
      target_rpm = atoi(buffer);
      pos = 0;
    } else if ('0' <= ch && ch <= '9') {
      buffer[pos++] = ch;
    }
  }

  analogWrite(laser_pwm_pin, (actual_rpm > 500) ? 24 : 0);

  delay(250);
}
