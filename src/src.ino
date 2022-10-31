#include <Arduino.h>

#include "FastLED.h" //https://github.com/FastLED/FastLED/
#include "timer.h"

const byte START_BUTTON_PIN = 12;
const byte STOP_BUTTON_PIN = 11;
const byte BAT_VOLT_PIN = A1;
const byte MOTOR_CTRL_PIN = 7;
const byte PIEZO_PIN = 4;
const byte BEAM_BREAK_PIN = 2;
const byte LEDS_DATA_PIN = 8;
const byte LEDS_CLOCK_PIN = 9;

const byte image_height = 8;
CRGB leds[image_height];

const int image_width = 100;
CRGB staged_image[image_width][image_height];
CRGB current_image[image_width][image_height];
volatile bool staged_image_new; // we want this to be atomic
volatile unsigned long last_rotation_micros; // interval
volatile unsigned long last_beam_break_micros;
volatile int column_counter;

enum State {
    MOTOR_OFF = 1,
    WAIT = 2,
    SPINNING_UP = 3,
    RUNNING = 4,
    SPINNING_DOWN = 5
};
volatile State state;

struct FsmInput {
    unsigned long micros;
    volatile bool start_button;
    volatile bool stop_button;
    bool rotation_interval;
    bool last_beam_break;
};
FsmInput fsm_input;

void setup()
{
    state = MOTOR_OFF;

    pinMode(START_BUTTON_PIN, INPUT_PULLUP);
    pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(BAT_VOLT_PIN, INPUT);
    pinMode(BEAM_BREAK_PIN, INPUT);

    analogWrite(MOTOR_CTRL_PIN, 0);

    FastLED.addLeds<APA102, LEDS_DATA_PIN, LEDS_CLOCK_PIN, BGR>(leds, image_height); // https://learn.sparkfun.com/tutorials/lumenati-hookup-guide#example-using-a-samd21-mini-breakout

    staged_image_new = false;
    last_rotation_micros = 0;
    column_counter = 0;
    last_beam_break_micros = micros();

    fsm_input.last_beam_break = last_beam_break_micros;
    fsm_input.micros = micros();
    fsm_input.rotation_interval = 0;
    fsm_input.start_button = false;
    fsm_input.stop_button = false;

    // setupTimer(); // TODO:

    attachInterrupt(BEAM_BREAK_PIN, beamBreakIsr, FALLING);
    attachInterrupt(START_BUTTON_PIN, startButtonISR, FALLING); // buttons pull pins low when pressed
    attachInterrupt(STOP_BUTTON_PIN, stopButtonISR, FALLING);
}
void loop()
{
    fsm_input.last_beam_break = last_beam_break_micros;
    fsm_input.micros = micros();
    fsm_input.rotation_interval = last_rotation_micros;
    fsm_input.start_button = false; // ISRs set it true
    fsm_input.stop_button = false;
    state = updateFSM(state, fsm_input);
}
State updateFSM(State state, FsmInput fsm_input)
{
    switch (state) {
    case State::MOTOR_OFF:
        if (fsm_input.start_button) { // transition 1-2
        }
        break;
    default:
        // invalid and theoretically unreachable state, stop the motor
        analogWrite(MOTOR_CTRL_PIN, 0);
        state = State::SPINNING_DOWN;
        break;
    }
    return state;
}
void beamBreakIsr()
{
    // speed
    unsigned long temp_micros = micros();
    last_rotation_micros = temp_micros - last_beam_break_micros;
    last_beam_break_micros = temp_micros;

    column_counter = 0;
    if (state == RUNNING) {
        if (staged_image_new) {
            stopTimerInterrupts();
            memcpy(current_image, staged_image, sizeof(CRGB) * image_width * image_height);
            setTimerISRRate((int32_t)image_width * 1000000 / last_rotation_micros);
        }
    }
}

void TC3_Handler()
{ // timerISR
    int temp_column_counter = constrain(column_counter, 0, image_width - 1);
    for (int i = 0; i < image_height; i++) {
        leds[i] = current_image[temp_column_counter][i];
    }
    FastLED.show();
    column_counter++;
}

void stopButtonISR()
{
    fsm_input.stop_button = true;
    state = updateFSM(state, fsm_input);
}
void startButtonISR()
{
    fsm_input.stop_button = false;
    state = updateFSM(state, fsm_input);
}
