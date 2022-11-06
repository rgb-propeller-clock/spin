#include <Arduino.h>

#include "FastLED.h" //https://github.com/FastLED/FastLED/
#include "font.h"
#include "timer.h"

const unsigned long wait_interval_micros = 2000000;
const unsigned long spin_down_time_micros = 1500000;
unsigned long speed_setpoint_k_rpm = 1000 * 12;
const unsigned long spinup_timeout = 5000000;
const unsigned int spinup_divider = 4000;

const byte START_BUTTON_PIN = 1;
const byte STOP_BUTTON_PIN = 0;
const byte BAT_VOLT_PIN = A1;
const byte MOTOR_CTRL_PIN = 7;
const byte PIEZO_PIN = 4;
const byte BEAM_BREAK_PIN = A2;
const byte LEDS_DATA_PIN = 8;
const byte LEDS_CLOCK_PIN = 9;

const byte image_height = 8;
CRGB leds[image_height];

const int image_width = 125;
CRGB staged_image[image_width][image_height] = { 0 };
CRGB current_image[image_width][image_height];
volatile bool staged_image_new; // we want this to be atomic
volatile unsigned long last_rotation_micros; // interval
volatile unsigned long last_beam_break_micros;
volatile int column_counter;
volatile unsigned long start_micros; // variable for FSM

enum State {
    s01_MOTOR_OFF = 1,
    s02_WAIT = 2,
    s03_SPINNING_UP = 3,
    s04_RUNNING = 4,
    s05_SPINNING_DOWN = 5
};
volatile State state;

struct FsmInput {
    volatile unsigned long micros;
    volatile bool start_button;
    volatile bool stop_button;
    volatile unsigned long rotation_interval;
    volatile unsigned long last_beam_break;
};
FsmInput fsm_input;

void setup()
{
    state = State::s01_MOTOR_OFF;

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

    setupTimer();

    attachInterrupt(BEAM_BREAK_PIN, beamBreakIsr, FALLING);
    attachInterrupt(START_BUTTON_PIN, startButtonIsr, FALLING); // buttons pull pins low when pressed
    attachInterrupt(STOP_BUTTON_PIN, stopButtonIsr, FALLING);
}

void loop()
{
    noInterrupts();
    fsm_input.last_beam_break = last_beam_break_micros;
    fsm_input.micros = micros();
    fsm_input.rotation_interval = last_rotation_micros;
    fsm_input.start_button = false; // ISRs set it true
    fsm_input.stop_button = false;
    state = updateFSM(state, fsm_input);
    interrupts();

    static long text_mover = 0;
    char text[20];
    sprintf(text, "Hello World!   %d", (int)((millis() / 1000) % 1000));
    printString(text, text_mover, CHSV(text_mover * 3, 255, 200), CRGB(0, 0, 0), staged_image, image_width);
    text_mover -= 2;

    for (int i = 0; i < image_width; i++) {
        staged_image[i][0] = CHSV(map(i, 0, image_width, 0, 255), 255, 100);
    }

    staged_image_new = true;

    delay(100);
}

State updateFSM(State state, FsmInput fsm_input)
{
    switch (state) {
    case State::s01_MOTOR_OFF:
        if (fsm_input.start_button) { // transition 1-2
            start_micros = micros();
            tone(PIEZO_PIN, 880);
            state = State::s02_WAIT;
        }
        return state;
        break;
    case State::s02_WAIT:
        if (fsm_input.stop_button) { // transition 2-1
            noTone(PIEZO_PIN);
            state = State::s01_MOTOR_OFF;
        } else if (fsm_input.micros - start_micros > wait_interval_micros) { // transition 2-3
            tone(PIEZO_PIN, 1320);
            last_rotation_micros = 0;
            start_micros = fsm_input.micros;
            state = State::s03_SPINNING_UP;
        }
        return state;
        break;
    case State::s03_SPINNING_UP:
        if (fsm_input.stop_button) { // transition 3-5a stop pressed
            analogWrite(MOTOR_CTRL_PIN, 0);
            tone(PIEZO_PIN, 1000);
            state = State::s05_SPINNING_DOWN;
        } else if (fsm_input.rotation_interval != 0 && fsm_input.rotation_interval <= 1000000 * 1000 / speed_setpoint_k_rpm) { // transition 3-4
            // fast enough
            noTone(PIEZO_PIN);
            state = State::s04_RUNNING;

        } else if (fsm_input.micros - start_micros > spinup_timeout) { // transition 3-5b timeout
            analogWrite(MOTOR_CTRL_PIN, 0);
            tone(PIEZO_PIN, 1000);
            state = State::s05_SPINNING_DOWN;
        } else {
            analogWrite(MOTOR_CTRL_PIN, constrain((fsm_input.micros - start_micros) / spinup_divider, 0, 255));
        }
        return state;
        break;
    case State::s04_RUNNING:
        if (fsm_input.stop_button) { // transition 4-5
            stopTimerInterrupts();
            analogWrite(MOTOR_CTRL_PIN, 0);
            tone(PIEZO_PIN, 1000);
            FastLED.clear(true);
            state = State::s05_SPINNING_DOWN;
        } else { // 4-4 self loop
            // TODO: speed control loop
            analogWrite(MOTOR_CTRL_PIN, 220);
        }
        return state;
        break;
    case State::s05_SPINNING_DOWN:
        if (fsm_input.micros - fsm_input.last_beam_break > spin_down_time_micros) {
            noTone(PIEZO_PIN);
            state = State::s01_MOTOR_OFF;
        }
        return state;
        break;
    default:
        // invalid and theoretically unreachable state, stop the motor
        analogWrite(MOTOR_CTRL_PIN, 0);
        stopTimerInterrupts();
        noTone(PIEZO_PIN);
        state = State::s05_SPINNING_DOWN;
        break;
        return state;
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
    if (state == s04_RUNNING) {
        if (staged_image_new) {
            stopTimerInterrupts();
            memcpy(current_image, staged_image, sizeof(CRGB) * image_width * image_height);
            staged_image_new = false;
        }
        setTimerISRRate((int32_t)image_width * 1000000 / last_rotation_micros); // TODO: ENSURE NOT OUT OF RANGE OR DIV/0 (minimum frequency is 30Hz)
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
    TC3->COUNT16.INTFLAG.reg |= TC_INTFLAG_MC0; // Clear interrupt register flag
}

void stopButtonIsr()
{
    fsm_input.last_beam_break = last_beam_break_micros;
    fsm_input.micros = micros();
    fsm_input.rotation_interval = last_rotation_micros;
    fsm_input.start_button = false;

    fsm_input.stop_button = true;
    fsm_input.micros = micros();
    state = updateFSM(state, fsm_input);
}

void startButtonIsr()
{
    fsm_input.last_beam_break = last_beam_break_micros;
    fsm_input.micros = micros();
    fsm_input.rotation_interval = last_rotation_micros;
    fsm_input.stop_button = false;

    fsm_input.start_button = true;
    state = updateFSM(state, fsm_input);
}
