#include <Arduino.h>

#include "FastLED.h" //https://github.com/FastLED/FastLED/

const byte nleds = 8;
const byte LEDS_DATA_PIN = 8;
const byte LEDS_CLOCK_PIN = 9;
CRGB leds[8];

const byte START_BUTTON_PIN = 12;
const byte STOP_BUTTON_PIN = 11;
const byte BAT_VOLT_PIN = A1;
const byte MOTOR_CTRL_PIN = 7;
const byte PIEZO_PIN = 4;
const byte BEAM_BREAK_PIN = 2;

bool going = false;
int speedCounter = 0;

void setup()
{
    Serial.begin(115200);

    FastLED.addLeds<APA102, LEDS_DATA_PIN, LEDS_CLOCK_PIN, BGR>(leds, nleds); // https://learn.sparkfun.com/tutorials/lumenati-hookup-guide#example-using-a-samd21-mini-breakout

    pinMode(START_BUTTON_PIN, INPUT_PULLUP);
    pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(BAT_VOLT_PIN, INPUT);
    pinMode(BEAM_BREAK_PIN, INPUT);
}
void loop()
{

    leds[2] = CRGB(0, 0, !digitalRead(BEAM_BREAK_PIN) * 255);
    leds[4] = CRGB((speedCounter > 255) * 250, (speedCounter > 255) * 250, (speedCounter > 255) * 250);
    leds[6] = CRGB(0, !digitalRead(START_BUTTON_PIN) * 25, 0);
    leds[7] = CRGB(!digitalRead(STOP_BUTTON_PIN) * 25, 0, 0);
    FastLED.show();

    if (!digitalRead(STOP_BUTTON_PIN)) {
        going = false;
    } else {
        if (!digitalRead(START_BUTTON_PIN)) {
            if (!going) {
                tone(PIEZO_PIN, 1000);
            }
            going = true;
        } else {
            noTone(PIEZO_PIN);
        }
    }

    if (going) {
        analogWrite(MOTOR_CTRL_PIN, constrain(speedCounter, 0, 255));
        speedCounter++;
        if (speedCounter > 400) {
            going = false;
        }
    } else {
        analogWrite(MOTOR_CTRL_PIN, 0);
        if (speedCounter != 0) {
        }
        speedCounter = 0;
    }
    delay(50);
}