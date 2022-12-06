FsmInput test_input;

void resetInput() {
  test_input.last_beam_break = micros();
  test_input.micros = micros();
  test_input.rotation_interval = 0;
  test_input.start_button = false;
  test_input.stop_button = false;
  test_input.bat_volt = 0;
  mock_led = Mock_Led::NONE;
  mock_builtin = Mock_Builtin::NONE;
  mock_tone = Mock_Tone::NONE;
  mock_motor = Mock_Motor::OFF;
}

void runAllTests() {
  bool passed = true;
  resetInput();
  State retval;
  State inputState;
  // Test 1-1
  inputState = State::s01_MOTOR_OFF;
  retval = updateFSM(inputState, test_input);
  if((retval != State::s01_MOTOR_OFF) or (mock_builtin != Mock_Builtin::BLINK)){
    Serial.println("Test 1-1 failed");
    Serial.println("Received state:");
    Serial.println(retval);
    Serial.println("Received builtin:");
    Serial.println(mock_builtin);
    Serial.println();
    passed = false;
  }
  resetInput();
  // Test 1-2
  inputState = State::s01_MOTOR_OFF;
  test_input.start_button = true;
  retval = updateFSM(inputState, test_input);
  if((retval != State::s02_WAIT) or (mock_tone != Mock_Tone::WAIT) or (mock_builtin != Mock_Builtin::OFF)){
    Serial.println("Test 1-2 failed");
    Serial.println("Received state:");
    Serial.println(retval);
    Serial.println("Received tone:");
    Serial.println(mock_tone);
    Serial.println("Received builtin:");
    Serial.println(mock_builtin);
    Serial.println();
    passed = false;
  }
  resetInput();
  // Test 2-1
  inputState = State::s02_WAIT;
  test_input.stop_button = true;
  retval = updateFSM(inputState, test_input);
  if((retval != State::s01_MOTOR_OFF) or (mock_tone != Mock_Tone::OFF)){
    Serial.println("Test 2-1 failed");
    Serial.println("Received state:");
    Serial.println(retval);
    Serial.println("Received tone:");
    Serial.println(mock_tone);
    Serial.println();
    passed = false;
  }
  resetInput();
  // Test 2-2
  inputState = State::s02_WAIT;
  test_input.micros = 1000;
  start_micros = 0;
  retval = updateFSM(inputState, test_input);
  if((retval != State::s02_WAIT) or (mock_led != Mock_Led::WARNING)){
    Serial.println("Test 2-2 failed");
    Serial.println("Received state:");
    Serial.println(retval);
    Serial.println("Received led:");
    Serial.println(mock_led);
    Serial.println();
    passed = false;
  }
  resetInput();
  // Test 2-3
  inputState = State::s02_WAIT;
  test_input.micros = 2000001;
  start_micros = 0;
  retval = updateFSM(inputState, test_input);
  if((retval != State::s03_SPINNING_UP) or (mock_tone != Mock_Tone::UP)){
    Serial.println("Test 2-3 failed");
    Serial.println("Received state:");
    Serial.println(retval);
    Serial.println("Received tone:");
    Serial.println(mock_tone);
    Serial.println();
    passed = false;
  }
  resetInput();
  // Test 3-3
  inputState = State::s03_SPINNING_UP;
  test_input.micros = 500;
  start_micros = 0;
  retval = updateFSM(inputState, test_input);
  if((retval != State::s03_SPINNING_UP) or (mock_led != Mock_Led::WARNING) or (mock_motor != Mock_Motor::RAMP)){
    Serial.println("Test 3-3 failed");
    Serial.println("Received state:");
    Serial.println(retval);
    Serial.println("Received led:");
    Serial.println(mock_led);
    Serial.println("Received motor:");
    Serial.println(mock_motor);
    Serial.println();
    passed = false;
  }
  resetInput();
  // Test 3-4  
  inputState = State::s03_SPINNING_UP;
  test_input.rotation_interval = 1;
  retval = updateFSM(inputState, test_input);
  if((retval != State::s04_RUNNING) or (mock_tone != Mock_Tone::OFF)){
    Serial.println("Test 3-4 failed");
    Serial.println("Received state:");
    Serial.println(retval);
    Serial.println("Received tone:");
    Serial.println(mock_tone);
    Serial.println();
    passed = false;
  }
  resetInput();
  // Test 3-5a
  inputState = State::s03_SPINNING_UP;
  test_input.stop_button = true;
  retval = updateFSM(inputState, test_input);
  if((retval != State::s05_SPINNING_DOWN) or (mock_tone != Mock_Tone::DOWN) or (mock_motor != Mock_Motor::OFF)){
    Serial.println("Test 3-5a failed");
    Serial.println("Received state:");
    Serial.println(retval);
    Serial.println("Received tone:");
    Serial.println(mock_tone);
    Serial.println("Received motor:");
    Serial.println(mock_motor);
    Serial.println();
    passed = false;
  }
  resetInput();
  // Test 3-5b
  inputState = State::s03_SPINNING_UP;
  test_input.micros = 6000000;
  start_micros = 0;
  retval = updateFSM(inputState, test_input);
  if((retval != State::s05_SPINNING_DOWN) or (mock_tone != Mock_Tone::DOWN) or (mock_motor != Mock_Motor::OFF)){
    Serial.println("Test 3-5b failed");
    Serial.println("Received state:");
    Serial.println(retval);
    Serial.println("Received tone:");
    Serial.println(mock_tone);
    Serial.println("Received motor:");
    Serial.println(mock_motor);
    Serial.println();
    passed = false;
  }
  resetInput();
  // Test 4-4
  inputState = State::s04_RUNNING;
  retval = updateFSM(inputState, test_input);
  if((retval != State::s04_RUNNING) or (mock_motor != Mock_Motor::ON)){
    Serial.println("Test 4-4 failed");
    Serial.println("Received state:");
    Serial.println(retval);
    Serial.println("Received motor:");
    Serial.println(mock_motor);
    Serial.println();
    passed = false;
  }
  resetInput();
  // Test 4-5
  inputState = State::s04_RUNNING;
  test_input.stop_button = true;
  retval = updateFSM(inputState, test_input);
  if((retval != State::s05_SPINNING_DOWN) or (mock_tone != Mock_Tone::DOWN) or (mock_motor != Mock_Motor::OFF) or (mock_led != Mock_Led::NONE)){
    Serial.println("Test 4-5 failed");
    Serial.println("Received state:");
    Serial.println(retval);
    Serial.println("Received tone:");
    Serial.println(mock_tone);
    Serial.println("Received led:");
    Serial.println(mock_led);
    Serial.println("Received motor:");
    Serial.println(mock_motor);
    Serial.println();
    passed = false;
  }
  resetInput();
  // Test 5-1
  inputState = State::s05_SPINNING_DOWN;
  test_input.micros = 1600000;
  test_input.last_beam_break = 500;
  retval = updateFSM(inputState, test_input);
  if((retval != State::State::s01_MOTOR_OFF) or (mock_tone != Mock_Tone::OFF) or (mock_led != Mock_Led::NONE)){
    Serial.println("Test 5-1 failed");
    Serial.println("Received state:");
    Serial.println(retval);
    Serial.println("Received tone:");
    Serial.println(mock_tone);
    Serial.println("Received led:");
    Serial.println(mock_led);
    Serial.println();
    passed = false;
  }
  resetInput();
  // Test 5-5
  inputState = State::s05_SPINNING_DOWN;
  test_input.micros = 1000;
  test_input.last_beam_break = 0;
  retval = updateFSM(inputState, test_input);
  if((retval != State::s05_SPINNING_DOWN) or (mock_led != Mock_Led::WARNING)){
    Serial.println("Test 5-5 failed");
    Serial.println("Received state:");
    Serial.println(retval);
    Serial.println("Received led:");
    Serial.println(mock_led);
    Serial.println();
    passed = false;
  }
  resetInput();
  
  if(passed){
    Serial.println("All tests passed!");
  } else{
    while(true);
  }
}
