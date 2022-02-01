// PixelClock
// Adrian McCarthy 2022

// The PixelClock fires an interrupt for each pixel around cone.
// It uses the processor's Timer/Counter2.

#ifndef PIXEL_CLOCK_H
#define PIXEL_CLOCK_H

class PixelClock {
  public:
    static void begin() {
      compute(640, 1800);
      start();
    }

    static void start() {
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

    static void stop() {
      TCCR2B = 0;  // change the clock source to none
      TIMSK2 = 0;  // disable the interrupt
    }

    static void resync() { TCNT2 = 0; }

    static void compute(long pix_per_rev, long rev_per_min) {
      Serial.print("For ");
      Serial.print(pix_per_rev);
      Serial.print(" pixels per revolution at ");
      Serial.print(rev_per_min);
      Serial.println(" RPM:");
      const float pixel_freq = rev_per_min * pix_per_rev / 60.0f /*seconds per minute*/;
      Serial.print("  We need a pixel timer frequency of ");
      Serial.print(pixel_freq);
      Serial.println(" Hz.");
      Serial.print("  prescaler * limit = ");
      Serial.print(F_CPU);
      Serial.print("Hz / ");
      Serial.print(pixel_freq);
      Serial.print("Hz = ");
      const float pre_times_lim = F_CPU / pixel_freq;
      Serial.println(pre_times_lim);
      constexpr long prescalers[] = { 0, 1, 8, 32, 64, 128, 256, 1024 };
      Serial.println("Solutions:");
      for (const auto &prescaler : prescalers) {
        if (prescaler == 0) continue;
        const long limit = static_cast<long>(pre_times_lim / prescaler + 0.5f);
        if (limit <= 0 || 255 < limit) continue;
        const float actual = static_cast<float>(F_CPU) / prescaler / limit;
        const float delta = actual - pixel_freq;
        Serial.print("  prescaler=");
        Serial.print(prescaler);
        Serial.print(" limit=");
        Serial.print(limit);
        Serial.print(" actual=");
        Serial.print(actual);
        Serial.print("Hz delta=");
        Serial.print(delta);
        Serial.println(" Hz");
      }
    }
};

#endif
