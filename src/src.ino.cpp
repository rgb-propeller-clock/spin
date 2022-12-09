# 1 "C:\\Users\\Joshua\\AppData\\Local\\Temp\\tmpjoggsuzb"
#include <Arduino.h>
# 1 "C:/Users/Joshua/Desktop/spin/src/src.ino"
#include <Arduino.h>


#ifdef RUN_UNIT_TESTS
#define MOCK_FUNCTIONS 
#endif

#define APPLICATION 1

#include "clock_time.h"
#include "font.h"
#include "fsm_types.h"
#include "pid.h"
#include "timer.h"
#include "unit_tests.h"
#include "watchdog.h"
#include <CircularBuffer.h>
#include <FastLED.h>
#include <SPI.h>

PID motorPid;

const unsigned long wait_interval_micros = 2000000;
const unsigned long spin_down_time_micros = 1500000;
const unsigned long spinup_timeout = 5000000;
const unsigned int spinup_divider = 2000;
const float bat_voltage_scaler = 0.01;
const float bat_voltage_low_thresh = 6.5;

const uint8_t speed_unit_devisor_power = 12;
const uint32_t speed_unit_devisor = (1 << speed_unit_devisor_power);
int32_t speed_setpoint = speed_unit_devisor * 10 / 1;

uint32_t too_slow_rotation_interval = 1000000 / 9;
uint32_t too_fast_rotation_interval = 1000000 / 13;

const byte START_BUTTON_PIN = 1;
const byte STOP_BUTTON_PIN = 0;
const byte BAT_VOLT_PIN = A1;
const byte MOTOR_CTRL_PIN = 7;
const byte PIEZO_PIN = 4;
const byte BEAM_BREAK_PIN = A2;
const byte LEDS_DATA_PIN = 8;
const byte LEDS_CLOCK_PIN = 9;
const byte IR_PIN = 5;

const byte image_height = 8;
CRGB leds[image_height];

const int image_width = 125;
CRGB staged_image[image_width][image_height] = { 0 };
CRGB current_image[image_width][image_height];
volatile bool staged_image_new;
volatile unsigned long last_rotation_micros;
volatile unsigned long last_beam_break_micros;
volatile int column_counter;
volatile unsigned long start_micros;

volatile unsigned long last_ir_micros;
volatile boolean ir_buf_lock = false;
int most_recent_ir_angle = -1;

CircularBuffer<int, 50> ir_buf;
void setup();
void loop();
State updateFSM(State state, FsmInput fsm_input);
void beamBreakIsr();
void TC3_Handler();
void stopButtonIsr();
void startButtonIsr();
int getIRAngle();
inline void turnOffMotor();
inline void playWaitingTone();
inline void playSpinningUpTone();
inline void playSpinningDownTone();
inline void stopPlayingTone();
inline void turnOffBuiltinLed();
inline void blinkBuiltinLed();
void dangerBlink();
void clearDisplay();
#line 65 "C:/Users/Joshua/Desktop/spin/src/src.ino"
void setup()
{
    state = State::s01_MOTOR_OFF;

    pinMode(START_BUTTON_PIN, INPUT_PULLUP);
    pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(BAT_VOLT_PIN, INPUT);
    pinMode(BEAM_BREAK_PIN, INPUT);

    analogWrite(MOTOR_CTRL_PIN, 0);

    FastLED.addLeds<APA102, LEDS_DATA_PIN, LEDS_CLOCK_PIN, BGR>(leds, image_height);

    motorPid = PID(0, 18, 100, 20, 0, 0, 255, speed_unit_devisor_power);

    staged_image_new = false;
    last_rotation_micros = 0;
    column_counter = 0;
    last_beam_break_micros = micros();

    fsm_input.last_beam_break = last_beam_break_micros;
    fsm_input.micros = micros();
    fsm_input.rotation_interval = 0;
    fsm_input.start_button = false;
    fsm_input.stop_button = false;
    fsm_input.bat_volt = 0;

    Serial.begin(115200);

#ifdef RUN_UNIT_TESTS
    while (!Serial)
        ;
    Serial.println("starting unit tests: ");
    delay(1000);
    runAllTests();
#endif

#if APPLICATION == 1
    leds[0] = CRGB(150, 180, 0);
    FastLED.show();
    getStartTime();
    leds[0] = CRGB(0, 0, 0);
    FastLED.show();

#endif

    clearDisplay();

    setupTimer();
    setupWatchdog();

    attachInterrupt(BEAM_BREAK_PIN, beamBreakIsr, FALLING);
    attachInterrupt(START_BUTTON_PIN, startButtonIsr, FALLING);
    attachInterrupt(STOP_BUTTON_PIN, stopButtonIsr, FALLING);
}

void loop()
{
    float bat_voltage = analogRead(BAT_VOLT_PIN) * bat_voltage_scaler;
    noInterrupts();
    fsm_input.bat_volt = bat_voltage;
    fsm_input.last_beam_break = last_beam_break_micros;
    fsm_input.micros = micros();
    fsm_input.rotation_interval = last_rotation_micros;
    fsm_input.start_button = false;
    fsm_input.stop_button = false;
    state = updateFSM(state, fsm_input);
    interrupts();

    int irAngle = getIRAngle();
    if (irAngle != -1) {
        most_recent_ir_angle = irAngle;
    }

#if APPLICATION == 0
    char text[20];
    clearDisplay();
    sprintf(text, "speed = %d", 1000000 / last_rotation_micros);
    printString(text, 0, CHSV(0, 0, 145), CRGB(0, 0, 0), staged_image, image_width);
#endif
#if APPLICATION == 1
    clearDisplay();
    char* text = getCurrentTime();
    printString(text, -millis() / 60, CHSV(millis() / 10, 255, 245), CRGB(0, 0, 0), staged_image, image_width);
#endif

#if APPLICATION == 2
    char text[20];
    if (most_recent_ir_angle == -1 || micros() - last_ir_micros > 5000000) {
        most_recent_ir_angle = -1;
        clearDisplay();
    } else {
        clearDisplay();
        sprintf(text, "%d", (int)((millis() / 1000) % 1000));
        printString(text, most_recent_ir_angle, CHSV(0, 0, 145), CRGB(0, 0, 0), staged_image, image_width);
        if (most_recent_ir_angle > 100) {
            delay(300);
        }
    }
#endif

    staged_image_new = true;

    petWatchdog();
    delay(100);
}

State updateFSM(State state, FsmInput fsm_input)
{
    switch (state) {
    case State::s01_MOTOR_OFF:
        if (fsm_input.start_button) {
            start_micros = micros();
            playWaitingTone();
            turnOffBuiltinLed();
            state = State::s02_WAIT;
        } else {
            blinkBuiltinLed();
        }
        return state;
        break;
    case State::s02_WAIT:
        if (fsm_input.stop_button) {
            stopPlayingTone();
            state = State::s01_MOTOR_OFF;
        } else if (fsm_input.micros - start_micros > wait_interval_micros) {
            playSpinningUpTone();
            last_rotation_micros = 0;
            start_micros = fsm_input.micros;
            state = State::s03_SPINNING_UP;
        } else {
            dangerBlink();
        }
        return state;
        break;
    case State::s03_SPINNING_UP:
        if (fsm_input.stop_button) {
            turnOffMotor();
            playSpinningDownTone();
            state = State::s05_SPINNING_DOWN;
        } else if (fsm_input.rotation_interval != 0 && fsm_input.rotation_interval <= (int64_t)1000000 * speed_unit_devisor / speed_setpoint) {

            stopPlayingTone();
            state = State::s04_RUNNING;
            ir_buf.clear();
            most_recent_ir_angle = -1;
            motorPid.initialize_time(fsm_input.micros);
        } else if (fsm_input.micros - start_micros > spinup_timeout) {
            turnOffMotor();
            playSpinningDownTone();
            state = State::s05_SPINNING_DOWN;
        } else {
            dangerBlink();
#ifdef MOCK_FUNCTIONS
            mock_motor = Mock_Motor::RAMP;
#else
            analogWrite(MOTOR_CTRL_PIN, constrain((fsm_input.micros - start_micros) / spinup_divider, 0, 255));
#endif
        }
        return state;
        break;
    case State::s04_RUNNING:
        if (fsm_input.stop_button
            || ((fsm_input.micros - fsm_input.last_beam_break) > too_slow_rotation_interval)
            || (fsm_input.rotation_interval > too_slow_rotation_interval)
            || (fsm_input.rotation_interval < too_fast_rotation_interval)
            || (fsm_input.bat_volt < bat_voltage_low_thresh)
        ) {
            stopTimerInterrupts();
            turnOffMotor();
            playSpinningDownTone();
            FastLED.clear(true);
            state = State::s05_SPINNING_DOWN;
        } else {
#ifdef MOCK_FUNCTIONS
            mock_motor = Mock_Motor::ON;
#else
            int32_t speed = (int64_t)1000000 * speed_unit_devisor / fsm_input.rotation_interval;
            int32_t motor_control = motorPid.calculate(speed_setpoint, speed, fsm_input.micros);
            analogWrite(MOTOR_CTRL_PIN, motor_control);
#endif
        }
        return state;
        break;
    case State::s05_SPINNING_DOWN:
        if (fsm_input.micros - fsm_input.last_beam_break > spin_down_time_micros) {
            stopPlayingTone();
            FastLED.clear(true);
            state = State::s01_MOTOR_OFF;
        } else {
            dangerBlink();
        }
        return state;
        break;
    default:

        analogWrite(MOTOR_CTRL_PIN, 0);
        stopTimerInterrupts();
        noTone(PIEZO_PIN);
        state = State::s05_SPINNING_DOWN;
        return state;
        break;
    }
    return state;
}




void beamBreakIsr()
{

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
        if (last_beam_break_micros == 0) {
            stopTimerInterrupts();
            return;
        }
        int32_t isrRate = (int32_t)image_width * 1000000 / last_rotation_micros;
        isrRate = max(30, isrRate);
        setTimerISRRate(isrRate);
    }
}




void TC3_Handler()
{
    int temp_column_counter = constrain(column_counter, 0, image_width - 1);
    if (digitalRead(IR_PIN) == LOW) {
        if (ir_buf_lock == false) {
            last_ir_micros = micros();
            ir_buf.push(temp_column_counter);
        }
    }
    for (int i = 0; i < image_height; i++) {
        leds[i] = current_image[temp_column_counter][i];
    }
    FastLED.show();
    column_counter++;
    TC3->COUNT16.INTFLAG.reg |= TC_INTFLAG_MC0;
}




void stopButtonIsr()
{
    fsm_input.last_beam_break = last_beam_break_micros;
    fsm_input.rotation_interval = last_rotation_micros;
    fsm_input.start_button = false;

    fsm_input.stop_button = true;
    fsm_input.micros = micros();
    state = updateFSM(state, fsm_input);
}




void startButtonIsr()
{
    fsm_input.last_beam_break = last_beam_break_micros;
    fsm_input.rotation_interval = last_rotation_micros;
    fsm_input.stop_button = false;

    fsm_input.start_button = true;
    fsm_input.micros = micros();
    state = updateFSM(state, fsm_input);
}





int getIRAngle()
{
    ir_buf_lock = true;
    int angle = -1;
    if (micros() - last_ir_micros > 400000) {
        if (ir_buf.size() >= 10) {

            long x = 0;
            long y = 0;
            for (int i = 0; i < ir_buf.size(); i++) {
                x += cos16((1 << 16) / image_width * ir_buf[i]);
                y += sin16((1 << 16) / image_width * ir_buf[i]);
            }
            if (x != 0 && y != 0) {
                angle = (atan2(-y, -x) + PI) * image_width / TWO_PI;
                angle = constrain(angle, 0, image_width - 1);
            }
        }
        ir_buf.clear();
    }
    ir_buf_lock = false;
    return angle;
}




inline void turnOffMotor()
{
#ifdef MOCK_FUNCTIONS
    mock_motor = Mock_Motor::OFF;
#else
    analogWrite(MOTOR_CTRL_PIN, 0);
#endif
}





inline void playWaitingTone()
{
#ifdef MOCK_FUNCTIONS
    mock_tone = Mock_Tone::WAIT;
#else
    tone(PIEZO_PIN, 880);
#endif
}




inline void playSpinningUpTone()
{
#ifdef MOCK_FUNCTIONS
    mock_tone = Mock_Tone::UP;
#else
    tone(PIEZO_PIN, 1320);
#endif
}




inline void playSpinningDownTone()
{
#ifdef MOCK_FUNCTIONS
    mock_tone = Mock_Tone::DOWN;
#else
    tone(PIEZO_PIN, 1000);
#endif
}



inline void stopPlayingTone()
{
#ifdef MOCK_FUNCTIONS
    mock_tone = Mock_Tone::OFF;
#else
    noTone(PIEZO_PIN);
#endif
}



inline void turnOffBuiltinLed()
{
#ifdef MOCK_FUNCTIONS
    mock_builtin = Mock_Builtin::OFF;
#else
    digitalWrite(LED_BUILTIN, LOW);
#endif
}



inline void blinkBuiltinLed()
{
#ifdef MOCK_FUNCTIONS
    mock_builtin = Mock_Builtin::BLINK;
#else
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
#endif
}




void dangerBlink()
{
#ifdef MOCK_FUNCTIONS
    mock_led = Mock_Led::WARNING;
#else
    static bool blinkVar = false;
    blinkVar = !blinkVar;
    for (int i = 0; i < image_height; i++) {
        leds[i] = (blinkVar) ? CRGB(255, 100, 0) : CRGB(0, 0, 0);
    }
    FastLED.show();
#endif
}




void clearDisplay()
{
    for (int x = 0; x < image_width; x++) {
        for (int y = 0; y < image_height; y++) {
            staged_image[x][y] = CRGB(0, 0, 0);
        }
    }
}