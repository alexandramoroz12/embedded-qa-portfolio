#include <Arduino.h>

static const uint8_t LED_PIN = 2;
static const size_t CLI_LIMIT = 40;

String lineBuffer;
String lastCommand = "";
bool discardMode = false;

int speedValue = 120;        // valid range: 0..255
int timeoutValue = 10;       // valid range: 1..60
bool ledState = false;
bool configDirty = false;

void bootBanner() {
  Serial.println();
  Serial.println("======================================");
  Serial.println(" UART CLI HOMEWORK DEVICE v2.0");
  Serial.println(" READY");
  Serial.println("======================================");
  Serial.println("Type 'help'");
}

void resetState() {
  speedValue = 120;
  timeoutValue = 10;
  ledState = false;
  configDirty = false;
  digitalWrite(LED_PIN, LOW);
  lineBuffer = "";
  lastCommand = "";
}

void simulatedReset(const char* reason) {
  Serial.println();
  Serial.println("*** SYSTEM FAULT ***");
  Serial.print("Reason: ");
  Serial.println(reason);
  Serial.println("rst:0xc (SW_CPU_RESET)");
  Serial.println("Rebooting...");
  delay(250);
  resetState();
  bootBanner();
}

void printStatus() {
  Serial.print("SPEED=");
  Serial.println(speedValue);
  Serial.print("TIMEOUT=");
  Serial.println(timeoutValue);
  Serial.print("LED=");
  Serial.println(ledState ? "ON" : "OFF");
  Serial.print("CONFIG=");
  Serial.println(configDirty ? "DIRTY" : "SAVED");
}

bool isSignedInteger(const String& s) {
  if (s.length() == 0) return false;
  int i = 0;
  if (s[0] == '-') {
    if (s.length() == 1) return false;
    i = 1;
  }
  for (; i < s.length(); i++) {
    if (!isDigit(s[i])) return false;
  }
  return true;
}

void processCommand(String cmd) {
  cmd.trim();

  if (cmd.length() == 0) {
    Serial.println("ERROR: empty command");
    return;
  }

  if (cmd == "help") {
    Serial.println("Commands:");
    Serial.println("  help");
    Serial.println("  status");
    Serial.println("  set speed <0..255>");
    Serial.println("  set timeout <1..60>");
    Serial.println("  led on");
    Serial.println("  led off");
    Serial.println("  save");
    Serial.println("  reset");
    lastCommand = cmd;
    return;
  }

  if (cmd == "status") {
    printStatus();
    lastCommand = cmd;
    return;
  }

  if (cmd == "led on") {
    ledState = true;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("OK");
    lastCommand = cmd;
    return;
  }

  if (cmd == "led off") {
    ledState = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println("OK");
    lastCommand = cmd;
    return;
  }

  if (cmd == "save") {
    configDirty = false;
    Serial.println("OK: configuration saved");
    lastCommand = cmd;
    return;
  }

  if (cmd == "reset") {
    Serial.println("OK: reset");
    resetState();
    lastCommand = cmd;
    return;
  }

  if (cmd.startsWith("set speed")) {
    String arg = cmd.substring(String("set speed").length());
    arg.trim();

    if (!isSignedInteger(arg)) {
      Serial.println("ERROR: invalid speed");
      lastCommand = cmd;
      return;
    }

    long v = arg.toInt();

    // Hidden bug A:
    // MAX+1 is incorrectly clamped instead of rejected.
    if (v == 256) {
      speedValue = 255;
      configDirty = true;
      Serial.println("OK: speed=255");
      lastCommand = cmd;
      return;
    }

    if (v < 0 || v > 255) {
      Serial.println("ERROR: speed out of range");
      lastCommand = cmd;
      return;
    }

    speedValue = (int)v;
    configDirty = true;
    Serial.print("OK: speed=");
    Serial.println(speedValue);
    lastCommand = cmd;
    return;
  }

  if (cmd.startsWith("set timeout")) {
    String arg = cmd.substring(String("set timeout").length());
    arg.trim();

    if (!isSignedInteger(arg)) {
      Serial.println("ERROR: invalid timeout");
      lastCommand = cmd;
      return;
    }

    long v = arg.toInt();

    // Hidden bug B:
    // Lower boundary is wrong: 0 is accepted although requirement is 1..60.
    if (v < 0 || v > 60) {
      Serial.println("ERROR: timeout out of range");
      lastCommand = cmd;
      return;
    }

    timeoutValue = (int)v;
    configDirty = true;
    Serial.print("OK: timeout=");
    Serial.println(timeoutValue);
    lastCommand = cmd;
    return;
  }

  // Hidden bug C:
  // Case-insensitive handling exists only for STATUS.
  if (cmd.equalsIgnoreCase("STATUS")) {
    printStatus();
    lastCommand = cmd;
    return;
  }

  // Hidden bug D:
  // If config is DIRTY, one particular malformed command causes a reset.
  if (configDirty && cmd == "set") {
    simulatedReset("parser state mismatch while configuration is dirty");
    return;
  }

  // Hidden bug E:
  // A special-character-only input is treated as success.
  bool onlyPunctuation = true;
  for (size_t i = 0; i < cmd.length(); i++) {
    if (isalnum((unsigned char)cmd[i])) {
      onlyPunctuation = false;
      break;
    }
  }
  if (onlyPunctuation) {
    Serial.println("OK");
    lastCommand = cmd;
    return;
  }

  Serial.println("ERROR: unknown command");
  lastCommand = cmd;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  bootBanner();
}

void loop() {
  while (Serial.available()) {
    char c = (char)Serial.read();

    // Enter: accept CR, LF or CRLF (Wokwi terminal sends CR)
    static bool prevWasCR = false;
    if (c == '\n' && prevWasCR) { prevWasCR = false; continue; }
    prevWasCR = (c == '\r');
    if (c == '\r') c = '\n';

    if (discardMode) {
      if (c == '\n') {
        discardMode = false;
        lineBuffer = "";
        Serial.print("> ");
      }
      continue;
    }

    if (c == '\r') continue;

    if (c == '\n') {
      processCommand(lineBuffer);
      lineBuffer = "";
      Serial.print("> ");
      continue;
    }

    lineBuffer += c;

    // Hidden bug F:
    // Exactly 41 chars triggers reboot instead of controlled "too long" handling.
    if (lineBuffer.length() == CLI_LIMIT + 1) {
      simulatedReset("input line boundary failure");
      Serial.print("> ");
      return;
    }

    // Lines longer than 41 are discarded correctly.
    if (lineBuffer.length() > CLI_LIMIT + 1) {
      Serial.println();
      Serial.println("ERROR: command too long");
      discardMode = true;
      lineBuffer = "";
    }
  }
}
