#ifndef PID_H
#define PID_H
#include <Arduino.h>
/**
 * @brief  A class implementing PID control.
 * @note   Uses 32 bit integer math instead of floats.
 */
class PID {
public:
    /**
     * constant offset to output
     */
    int32_t K;
    /**
     * feedforward term (output += F*setpoint)
     */
    int32_t F;
    int32_t P;
    int32_t I;
    int32_t D;

    int32_t out_low;
    int32_t out_high;
    uint8_t out_devisor_pow;

    int32_t sum_error;
    int32_t last_calc_micros;
    int32_t last_error;

public:
    PID()
    {
        sum_error = 0;
        last_calc_micros = 0;
        last_error = 0;

        K = 0;
        F = 0;
        P = 0;
        I = 0;
        D = 0;
        out_low = 0;
        out_high = 0;
        out_devisor_pow = 0;
    }
    PID(int32_t k, int32_t f, int32_t p, int32_t i, int32_t d, int32_t _out_low, int32_t _out_high, uint8_t _out_devisor_pow)
    {
        PID();
        K = k;
        F = f;
        P = p;
        I = i;
        D = d;
        out_low = _out_low;
        out_high = _out_high;
        out_devisor_pow = _out_devisor_pow;
    }
    void initialize_time(unsigned long micros)
    {
        last_calc_micros = micros;
    }
    int32_t calculate(int32_t setpoint, int32_t input, unsigned long micros)
    {
        unsigned long dt = micros - last_calc_micros;
        last_calc_micros = micros;

        int32_t error = setpoint - input;

        sum_error += I * error * (int32_t)dt / 1000000;
        sum_error = constrain(sum_error, -(int32_t)(1 << out_devisor_pow) * (out_high - out_low) / 2, (int32_t)(1 << out_devisor_pow) * (out_high - out_low) / 2); // anti windup

        int64_t output = P * (error) + (sum_error) + D * (error - last_error) / (int32_t)dt; // PID

        output += K + setpoint * F; // add constant offset, and feedforward terms

        last_error = error;

        return constrain((int32_t)((int32_t)output / (int32_t)(1 << out_devisor_pow)), out_low, out_high);
    }
};

#endif // PID_H
