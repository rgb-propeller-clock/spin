void setupWatchdog() {
    // Set up NVIC: [SECTION 1]
    NVIC_SetPriority(TC3_IRQn, 0);
    NVIC_EnableIRQ(TC3_IRQn);

    // Clear and enable WDT [SECTION 2]
    NVIC_DisableIRQ(WDT_IRQn);
    NVIC_ClearPendingIRQ(WDT_IRQn);
    NVIC_SetPriority(WDT_IRQn, 0);
    NVIC_EnableIRQ(WDT_IRQn);

    // TODO: Configure and enable WDT GCLK: [SECTION 3]
    GCLK->GENDIV.reg = GCLK_GENDIV_DIV(4) | GCLK_GENDIV_ID(5);
    while (GCLK->STATUS.bit.SYNCBUSY);
    // set GCLK->GENCTRL.reg and GCLK->CLKCTRL.reg
    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_ID(5) | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_DIVSEL;
    while(GCLK->STATUS.bit.SYNCBUSY);

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(5) | GCLK_CLKCTRL_ID(3);
    while(GCLK->STATUS.bit.SYNCBUSY);

    // TODO: Configure and enable WDT: [SECTION 4]
    /* we configured the clock 5 to be at 1kHz
    * set it to (~1/4 seconds according to our 1kHz clock) 256 clock cycles according to (http://ww1.microchip.com/downloads/en/DeviceDoc/SAM_D21_DA1_Family_DataSheet_DS40001882F.pdf)
    */
    WDT->CONFIG.reg = 5; // 0.25 seconds
    WDT->EWCTRL.reg = 4; // early warning 0.125 seconds
    WDT->CTRL.reg = WDT_CTRL_ENABLE;

    // TODO: Enable early warning interrupts on WDT:
    // reference WDT registers with WDT->register_name.reg
    WDT->INTENSET.reg |= WDT_INTENSET_EW;
    while(WDT->STATUS.bit.SYNCBUSY);
}

void petWatchdog() {
    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
}

/**
  * This function is called when the watchdog is not pet
  */
void WDT_Handler() {
  // TODO: Clear interrupt register flag
  // (reference register with WDT->register_name.reg)
  WDT->INTFLAG.reg = WDT_INTFLAG_EW;
  // TODO: Warn user that a watchdog reset may happen
  Serial.println("ERROR: Did not pet watchdog!");
  // ASK: Do we want to stop the clock at this point?
}
