#include <Arduino.h>

// Timer code -------------------------------- //
const int CLOCKFREQ = 4000000;

void setupTimer() {
  /*
   * LAB STEP 4
   */
  // TODO: Configure and enable GCLK4 for TC:
  GCLK->GENDIV.reg = GCLK_GENDIV_DIV(0) | GCLK_GENDIV_ID(4); // do not divide gclk 4
  while(GCLK->STATUS.bit.SYNCBUSY);
  // use GCLK->GENCTRL.reg and GCLK->CLKCTRL.reg
  GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_IDC | GCLK_GENCTRL_ID(4) | GCLK_GENCTRL_SRC_OSC8M;
  while(GCLK->STATUS.bit.SYNCBUSY);
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(27) | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(4);
  
  /*
   * LAB STEP 6
   */
  // TODO: Disable TC (for now)
  // use TC3->COUNT16.CTRLA.reg and TC3->COUNT16.INTENCLR.reg
  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY);

  TC3->COUNT16.INTENCLR.reg |= TC_INTENCLR_MC0;

  // Set up NVIC:
  NVIC_SetPriority(TC3_IRQn, 0);
  NVIC_EnableIRQ(TC3_IRQn);

  Serial.println("Timer Initialized!");
}

/*
 * Sets correct TC timer value and enables TC interrupt at the desired frequency
 * Non-blocking: TC interrupts can happen while program executes
 */
void setTimerISRRate(int freq) {
  // Reference TC with TC3->COUNT16.register_name.reg
  // TODO: Turn off interrupts to TC3 on MC0 when configuring
  TC3->COUNT16.INTENCLR.reg |= TC_INTENCLR_MC0;
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY); // might not need

  TC3->COUNT16.CTRLA.reg = TC_CTRLA_ENABLE | TC_CTRLA_MODE(0) | TC_CTRLA_PRESCALER(1) | TC_CTRLA_PRESCSYNC(1) | TC_CTRLA_WAVEGEN(1);
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY); // might not need

  TC3->COUNT16.CC[0].reg = CLOCKFREQ/freq;
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY);

  // TODO: Turn interrupts to TC3 on MC0 back on when done configuring
  TC3->COUNT16.INTENSET.reg |= TC_INTENSET_MC0;
}

/*
 * Disables TC timer
 */
void stopTimerInterrupts() {
  // TODO: Reference TC with TC3->COUNT16.register_name.reg
  TC3->COUNT16.INTENCLR.reg |= TC_INTENCLR_MC0;
  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while(TC3->COUNT16.STATUS.bit.SYNCBUSY);
  Serial.println("Timer interrupts stopped!");
}
