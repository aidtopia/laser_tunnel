#include <Arduino.h>
#include "aidassert.h"

#ifndef NDEBUG

void assertionFailure(const __FlashStringHelper *cond, const __FlashStringHelper *file, int line) {
  if (Serial) {
    Serial.print(file);
    Serial.print('(');
    Serial.print(line);
    Serial.print(F("): "));
    Serial.print(F("assertion failure: `"));
    Serial.print(cond);
    Serial.println('`');
  }
  for (;;) { delay(1000); }
}

#endif
