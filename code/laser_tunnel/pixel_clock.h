// PixelClock
// Adrian McCarthy 2022

// The PixelClock fires an interrupt for each pixel around the cone.
// It uses the processor's Timer/Counter2.

#ifndef PIXEL_CLOCK_H
#define PIXEL_CLOCK_H

class PixelClock {
  public:
    static void begin(float freq) { start(freq); }
    static void begin(uint8_t prescaler_index, uint8_t limit) {
      start(prescaler_index, limit);
    }

    static void start(uint8_t prescaler_index, uint8_t limit) {
      // Set the wave generation mode (3 bits across two registers) to
      // compare timer/counter (CTC) to OCR2A.
      TCCR2A = (TCCR2A & 0b11111100) | (1 << WGM21);
      TCCR2B = (TCCR2B & 0b11110111);
      // Set the clock source prescaler.
      TCCR2B = (TCCR2B & 0b11111000) | (prescaler_index & 0b00000111);
      TCNT2 = 0;  // start counting from 0.
      OCR2A = limit;  // count to the limit
      TIMSK2 |= (1 << OCIE2A);  // enable interrupt each time the counter reaches the limit
    }

    static void start(float freq) {
      Serial.print("  prescaler * limit = ");
      Serial.print(F_CPU);
      Serial.print("Hz / ");
      Serial.print(freq);
      Serial.print("Hz = ");
      const float pre_times_lim = F_CPU / freq;
      Serial.println(pre_times_lim);

      constexpr long prescalers[] = { 0, 1, 8, 32, 64, 128, 256, 1024 };
      constexpr auto prescaler_count =
        static_cast<uint8_t>(sizeof(prescalers)/sizeof(prescalers[0]));
      for (uint8_t i = 0; i < prescaler_count; ++i) {
        const auto prescaler = prescalers[i];
        if (prescaler == 0) continue;
        const long limit = static_cast<long>(pre_times_lim / prescaler + 0.5f);
        if (limit <= 0 || 255 < limit) continue;
        const float actual = static_cast<float>(F_CPU) / prescaler / limit;
        const float delta = actual - freq;
        Serial.print("  prescaler=");
        Serial.print(prescaler);
        Serial.print(", limit=");
        Serial.print(limit);
        Serial.print(", actual=");
        Serial.print(actual);
        Serial.print(" Hz, delta=");
        Serial.print(delta);
        Serial.println(" Hz");
        return start(i, limit);
      }
      Serial.println("No solution for that frequency.");
      stop();
    }

    static void stop() {
      TCCR2B = 0;  // change the clock source to none
      TIMSK2 = 0;  // disable the interrupt
    }

    static void resync() { TCNT2 = 0; }

};

#endif
