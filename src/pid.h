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
    /**
     * proportional term
     */
    int32_t P;
    /**
     * integral term
     */
    int32_t I;
    /**
     * derivitive term
     */
    int32_t D;

    int32_t out_low;
    int32_t out_high;
    uint8_t out_devisor_pow;

    int32_t sum_error;
    int32_t last_calc_micros;
    int32_t last_error;

public:
    /**
     * @brief  default constructor for PID class, use the one that takes parameters
     */
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
    /**
     * @brief  constructor for PID class
     * @param  k: constant offset to output
     * @param  f: feedforward term (output += F*setpoint)
     * @param  p: proportional
     * @param  i: integral
     * @param  d: derivitive
     * @param  _out_low: low bound of output
     * @param  _out_high: high bound of output
     * @param  _out_devisor_pow: output gets divided by 2^_out_devisor_pow
     */
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
    /**
     * @brief  call this when starting the pid loop (to avoid the loop thinking there was a huge time step at the first calculate)
     */
    void initialize_time(unsigned long micros)
    {
        last_calc_micros = micros;
    }
    /**
     * @brief  Call this method to run the PID loop
     * @param  setpoint: (int32_t) target value to reach
     * @param  input: (int32_t) input (measurement)
     * @param  micros: (unsigned long) a value that increases steadily at a rate of one million per second.
     * @retval (int32_t) output value of PID control loop
     */
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
