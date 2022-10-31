#include <Arduino.h>

#include "FastLED.h" //https://github.com/FastLED/FastLED/

const byte image_height = 8;
const byte LEDS_DATA_PIN = 8;
const byte LEDS_CLOCK_PIN = 9;
CRGB leds[image_height];

const byte START_BUTTON_PIN = 12;
const byte STOP_BUTTON_PIN = 11;
const byte BAT_VOLT_PIN = A1;
const byte MOTOR_CTRL_PIN = 7;
const byte PIEZO_PIN = 4;
const byte BEAM_BREAK_PIN = 2;

void setup()
{
    Serial.begin(115200);

    FastLED.addLeds<APA102, LEDS_DATA_PIN, LEDS_CLOCK_PIN, BGR>(leds, image_height); // https://learn.sparkfun.com/tutorials/lumenati-hookup-guide#example-using-a-samd21-mini-breakout

    pinMode(START_BUTTON_PIN, INPUT_PULLUP);
    pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(BAT_VOLT_PIN, INPUT);
    pinMode(BEAM_BREAK_PIN, INPUT);
}
void loop()
{
    !digitalRead(BEAM_BREAK_PIN);
    !digitalRead(START_BUTTON_PIN);
    !digitalRead(STOP_BUTTON_PIN);
}