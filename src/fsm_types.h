#ifndef FSM_TYPES_H
#define FSM_TYPES_H
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
    volatile float bat_volt;
};
FsmInput fsm_input;

#endif // FSM_TYPES_H
