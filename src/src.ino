#include <Arduino.h>

#include "FastLED.h" //https://github.com/FastLED/FastLED/

const byte nleds = 8;
const byte LEDS_DATA_PIN = 8;
const byte LEDS_CLOCK_PIN = 9;
CRGB leds[8];

void setup()
{
    Serial.begin(115200);

    FastLED.addLeds<APA102, LEDS_DATA_PIN, LEDS_CLOCK_PIN, BGR>(leds, nleds); // https://learn.sparkfun.com/tutorials/lumenati-hookup-guide#example-using-a-samd21-mini-breakout
}

void loop()
{
    static int count = 0;
    static unsigned long micros_start = micros();
    for (byte i = 0; i < nleds; i++) {
        leds[i] = CRGB(255 * ((count + i) % 3 == 0), 255 * ((count + i) % 3 == 1), 255 * ((count + i) % 3 == 2));
    }
    FastLED.show();
    count++;
    if (count >= 1000) {
        Serial.print("microseconds: ");
        Serial.print(micros() - micros_start);
        Serial.print(", updates: ");
        Serial.print(count);
        Serial.print(", updates per second: ");
        Serial.print(count / (double)((double)(micros() - micros_start) / 1000000.0));
        Serial.println();
        count = 0;
        micros_start = micros();
    }
}
