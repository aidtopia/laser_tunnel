// Interface for Arduino's timer/counters
// Adrian McCarthy 2022

#ifndef TIMERS_H
#define TIMERS_H

template <int N>
class Timer {
  public:
    void begin(float freq) { start(freq); }
    void begin(uint8_t prescaler_index, uint8_t limit) {
      start(prescaler_index, limit);
    }

    void start(uint8_t prescaler_index, uint8_t limit);
    void start(float freq) {
      Serial.print("  prescaler * limit = ");
      Serial.print(F_CPU);
      Serial.print("Hz / ");
      Serial.print(freq);
      Serial.print("Hz = ");
      const float pre_times_lim = F_CPU / freq;
      Serial.println(pre_times_lim);

      constexpr auto prescaler_count =
        static_cast<uint8_t>(sizeof(prescalers)/sizeof(prescalers[0]));
      for (uint8_t i = 0; i < prescaler_count; ++i) {
        const auto prescaler = prescalers[i];
        if (prescaler == 0) continue;
        const long limit = static_cast<long>(pre_times_lim / prescaler + 0.5f);
        if (limit < 1 || 255 < limit) continue;
        const float actual = static_cast<float>(F_CPU) / prescaler / limit;
        const float delta = actual - freq;
        Serial.print(F("  prescaler="));
        Serial.print(prescaler);
        Serial.print(F(", limit="));
        Serial.print(limit);
        Serial.print(F(", actual="));
        Serial.print(actual);
        Serial.print(F(" Hz, delta="));
        Serial.print(delta);
        Serial.println(F(" Hz"));
        return start(i, limit);
      }
      Serial.println(F("No solution for that frequency."));
      stop();
    }

    void stop();
    void resync();

  private:
    static const long prescalers[8];
};

template <>
void Timer<2>::start(uint8_t prescaler_index, uint8_t limit) {
  // Set the wave generation mode (3 bits across two registers) to
  // compare timer/counter (CTC) to OCR2A.
  TCCR2A = (TCCR2A & 0b11111100) | (1 << WGM21);
  TCCR2B = (TCCR2B & 0b11110111);
  // Set the clock source prescaler.
  TCCR2B = (TCCR2B & 0b11111000) | (prescaler_index & 0b00000111);
  TCNT2 = 0;  // start counting from 0.
  OCR2A = limit - 1;  // count up to the limit
  TIMSK2 |= (1 << OCIE2A);  // enable interrupt each time the counter reaches the limit
}

template <>
void Timer<2>::stop() {
  TCCR2B = 0;  // change the clock source to none
  TIMSK2 = 0;  // disable the interrupt
}

template <>
void Timer<2>::resync() { TCNT2 = 0; }

template <>
const long Timer<2>::prescalers[8] = { 0, 1, 8, 32, 64, 128, 256, 1024 };

#endif
