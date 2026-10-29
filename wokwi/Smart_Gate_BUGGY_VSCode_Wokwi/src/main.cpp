#include <Arduino.h>

/*
  QA Embedded Training Project
  SMART GATE CONTROLLER — BUGGY TRAINING VERSION

  IMPORTANT:
  This version intentionally contains hidden defects for QA practice.
  Students receive requirements + this project and must discover the bugs.

  Board: Arduino Uno

  INPUTS
    D2  OPEN button
    D3  CLOSE button
    D4  STOP button
    D5  OPEN limit switch
    D6  CLOSE limit switch

  OUTPUTS
    D8  OPEN relay
    D9  CLOSE relay
    D10 OPEN LED
    D11 CLOSE LED
    D12 ERROR LED
    D13 BUZZER

  Serial commands:
    O = OPEN
    C = CLOSE
    S = STOP
    R = RESET ERROR
*/

enum GateState {
  CLOSED,
  OPENING,
  OPEN,
  CLOSING,
  STOPPED,
  ERROR_STATE
};

const byte BTN_OPEN   = 2;
const byte BTN_CLOSE  = 3;
const byte BTN_STOP   = 4;
const byte LIMIT_OPEN = 5;
const byte LIMIT_CLOSE = 6;

const byte RELAY_OPEN  = 8;
const byte RELAY_CLOSE = 9;
const byte LED_OPEN    = 10;
const byte LED_CLOSE   = 11;
const byte LED_ERROR   = 12;
const byte BUZZER      = 13;

const unsigned long MOVE_TIMEOUT = 10000UL;
const unsigned long DEBOUNCE_MS = 60UL;

GateState state = CLOSED;
unsigned long movementStartedAt = 0;

bool lastOpen = HIGH;
bool lastClose = HIGH;
bool lastStop = HIGH;
unsigned long lastDebounceOpen = 0;
unsigned long lastDebounceClose = 0;
unsigned long lastDebounceStop = 0;

const char* stateName(GateState s) {
  switch (s) {
    case CLOSED: return "CLOSED";
    case OPENING: return "OPENING";
    case OPEN: return "OPEN";
    case CLOSING: return "CLOSING";
    case STOPPED: return "STOPPED";
    case ERROR_STATE: return "ERROR";
  }
  return "UNKNOWN";
}

void allRelaysOff() {
  digitalWrite(RELAY_OPEN, LOW);
  digitalWrite(RELAY_CLOSE, LOW);
}

void updateOutputs() {
  allRelaysOff();

  digitalWrite(LED_OPEN, LOW);
  digitalWrite(LED_CLOSE, LOW);
  digitalWrite(LED_ERROR, LOW);

  switch (state) {
    case CLOSED:
      break;

    case OPENING:
      digitalWrite(RELAY_OPEN, HIGH);
      digitalWrite(LED_OPEN, HIGH);
      break;

    case OPEN:
      digitalWrite(LED_OPEN, HIGH);
      break;

    case CLOSING:
      digitalWrite(RELAY_CLOSE, HIGH);
      digitalWrite(LED_CLOSE, HIGH);
      break;

    case STOPPED:
      // BUG #1 candidate: STOPPED is intentionally displayed as OPEN.
      digitalWrite(LED_OPEN, HIGH);
      break;

    case ERROR_STATE:
      digitalWrite(LED_ERROR, HIGH);
      tone(BUZZER, 1200);
      return;
  }

  noTone(BUZZER);
}

void changeState(GateState newState) {
  if (state != newState) {
    Serial.print("STATE: ");
    Serial.print(stateName(state));
    Serial.print(" -> ");
    Serial.println(stateName(newState));
  }

  state = newState;

  if (state == OPENING || state == CLOSING) {
    movementStartedAt = millis();
  }

  updateOutputs();
}

void stopGate() {
  // BUG #2: STOP changes state but does not immediately switch outputs off.
  changeState(STOPPED);
}

bool buttonPressed(byte pin, bool &lastReading, unsigned long &lastDebounce) {
  bool reading = digitalRead(pin);

  if (reading != lastReading) {
    lastDebounce = millis();
    lastReading = reading;
  }

  if ((millis() - lastDebounce) > DEBOUNCE_MS && reading == LOW) {
    while (digitalRead(pin) == LOW) {
      delay(1);
    }
    lastReading = HIGH;
    return true;
  }

  return false;
}

void handleCommand(char cmd) {
  cmd = toupper(cmd);

  Serial.print("COMMAND: ");
  Serial.println(cmd);

  if (cmd == 'S') {
    stopGate();
    return;
  }

  if (cmd == 'R') {
    if (state == ERROR_STATE) {
      changeState(STOPPED);
    }
    return;
  }

  if (state == ERROR_STATE) {
    Serial.println("IGNORED: controller is in ERROR");
    return;
  }

  if (cmd == 'O') {
    // BUG #3: OPEN is accepted during CLOSING without first stopping.
    if (state == CLOSED || state == STOPPED || state == CLOSING) {
      if (digitalRead(LIMIT_OPEN) == LOW) {
        changeState(OPEN);
      } else {
        changeState(OPENING);
      }
    } else {
      Serial.println("IGNORED: OPEN is not valid in current state");
    }
  }

  if (cmd == 'C') {
    // BUG #4: CLOSE is accepted during OPENING.
    if (state == OPEN || state == STOPPED || state == OPENING) {
      if (digitalRead(LIMIT_CLOSE) == LOW) {
        changeState(CLOSED);
      } else {
        changeState(CLOSING);
      }
    } else {
      Serial.println("IGNORED: CLOSE is not valid in current state");
    }
  }

  // BUG #5: invalid commands are silently treated as harmless,
  // but the requirement expects an explicit ERROR indication.
  if (cmd != 'O' && cmd != 'C') {
    Serial.println("UNKNOWN COMMAND");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN_OPEN, INPUT_PULLUP);
  pinMode(BTN_CLOSE, INPUT_PULLUP);
  pinMode(BTN_STOP, INPUT_PULLUP);
  pinMode(LIMIT_OPEN, INPUT_PULLUP);
  pinMode(LIMIT_CLOSE, INPUT_PULLUP);

  pinMode(RELAY_OPEN, OUTPUT);
  pinMode(RELAY_CLOSE, OUTPUT);
  pinMode(LED_OPEN, OUTPUT);
  pinMode(LED_CLOSE, OUTPUT);
  pinMode(LED_ERROR, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  allRelaysOff();
  updateOutputs();

  Serial.println("=== SMART GATE CONTROLLER ===");
  Serial.println("Commands: O=open, C=close, S=stop, R=reset error");
  Serial.println("Initial state: CLOSED");
}

void loop() {
  if (Serial.available()) {
    handleCommand(Serial.read());
  }

  if (buttonPressed(BTN_OPEN, lastOpen, lastDebounceOpen)) {
    handleCommand('O');
  }

  if (buttonPressed(BTN_CLOSE, lastClose, lastDebounceClose)) {
    handleCommand('C');
  }

  if (buttonPressed(BTN_STOP, lastStop, lastDebounceStop)) {
    handleCommand('S');
  }

  // BUG #6: OPENING does not stop at LIMIT_OPEN; it goes to ERROR instead.
  if (state == OPENING && digitalRead(LIMIT_OPEN) == LOW) {
    Serial.println("OPEN LIMIT DETECTED");
    // intentionally missing: changeState(OPEN);
  }

  if (state == CLOSING && digitalRead(LIMIT_CLOSE) == LOW) {
    changeState(CLOSED);
  }

  // BUG #7: simultaneous relay detection is missing.

  // BUG #8: timeout is 10 seconds as required, but only CLOSING is checked.
  if (state == CLOSING &&
      millis() - movementStartedAt > MOVE_TIMEOUT) {
    Serial.println("ERROR: movement timeout");
    changeState(ERROR_STATE);
  }

  // BUG #9: if both limit switches are active, controller does not enter ERROR.
  if (digitalRead(LIMIT_OPEN) == LOW && digitalRead(LIMIT_CLOSE) == LOW) {
    Serial.println("WARNING: both limit switches active");
    // intentionally no state change
  }
}
