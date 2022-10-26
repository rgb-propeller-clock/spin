#include <Arduino.h>

#include "FastLED.h" //https://github.com/FastLED/FastLED/

// this program updates the leds at 1000hz on a timer, with every 10 updates green and the rest red


const byte nleds = 8;
const byte LEDS_DATA_PIN = 8;
const byte LEDS_CLOCK_PIN = 9;
CRGB leds[8];

volatile int count = 0;
const int CLOCKFREQ = 8000000 / 2;

void setup()
{
    Serial.begin(115200);

    FastLED.addLeds<APA102, LEDS_DATA_PIN, LEDS_CLOCK_PIN, BGR>(leds, nleds); // https://learn.sparkfun.com/tutorials/lumenati-hookup-guide#example-using-a-samd21-mini-breakout
    /*
     * LAB STEP 4
     */
    // Configure and enable GCLK4 for TC:
    GCLK->GENDIV.reg = GCLK_GENDIV_DIV(0) | GCLK_GENDIV_ID(4); // do not divide gclk 4
    while (GCLK->STATUS.bit.SYNCBUSY)
        ;
    // use GCLK->GENCTRL.reg and GCLK->CLKCTRL.reg
    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_ID(4) | GCLK_GENCTRL_IDC | GCLK_GENCTRL_SRC_OSC8M;
    while (GCLK->STATUS.bit.SYNCBUSY)
        ;
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(4) | GCLK_CLKCTRL_ID_TCC2_TC3;

    /*
     * LAB STEP 5
     */
    // Check if APB is enabled:
    // Serial.println(PM->APBCMASK.reg, HEX); //11th bit is a 1 !

    /*
     * LAB STEP 6
     */
    // Disable TC (for now)
    // use TC3->COUNT16.CTRLA.reg and TC3->COUNT16.INTENCLR.reg
    TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY)
        ;
    TC3->COUNT16.INTENCLR.reg |= TC_INTENCLR_MC0;

    // Set up NVIC:
    NVIC_SetPriority(TC3_IRQn, 0);
    NVIC_EnableIRQ(TC3_IRQn);

    play_note(1000);
}
/*
 * Sets correct TC timer value and enables TC interrupt so that the interrupt toggles the output pin
 * at the desired frequency
 * Non-blocking: note can play while program executes
 */
void play_note(int freq)
{
    // Reference TC with TC3->COUNT16.register_name.reg

    // Turn off interrupts to TC3 on MC0 when configuring
    // Disable TC (for now)
    TC3->COUNT16.INTENCLR.reg |= TC_INTENCLR_MC0;

    // CONFIGURE TC3 TO PLAY NOTE

    TC3->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_PRESCALER_DIV2 | TC_CTRLA_PRESCSYNC_PRESC | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_ENABLE;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY)
        ;
    TC3->COUNT16.CC[0].reg = CLOCKFREQ / (freq); // it was /2
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY)
        ;

    // Turn interrupts to TC3 on MC0 back on when done configuring
    TC3->COUNT16.INTENSET.reg |= TC_INTENSET_MC0;
}
void loop()
{
    delay(1000);
}

void TC3_Handler()
{
    // Clear interrupt register flag
    // (use register TC3->COUNT16.register_name.reg)
    TC3->COUNT16.INTFLAG.reg |= TC_INTFLAG_MC0;

    count++;
    for (byte i = 0; i < nleds; i++) {
        leds[i] = CRGB(50, 255 * ((count % 10) == 0), 0);
    }
    FastLED.show();
}