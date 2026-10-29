#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 18;
constexpr uint8_t LED_PIN = 2;

// Software debounce configuration
constexpr uint32_t DEBOUNCE_US = 1000;

volatile uint32_t rawInterrupts = 0;
volatile uint32_t lastEdgeUs = 0;
volatile bool edgePending = false;

uint32_t logicalEvents = 0;
uint32_t actionCount = 0;
uint32_t lastLoggedRawInterrupts = 0;
int stableState = HIGH;
bool ledState = false;

uint32_t operationUntil = 0;

// --------------------------------------------------
// ISR
// --------------------------------------------------
void IRAM_ATTR buttonISR() {
  rawInterrupts++;
  lastEdgeUs = micros();
  edgePending = true;
}

// --------------------------------------------------
// Accepted logical event
// --------------------------------------------------
void acceptButtonPress() {
  logicalEvents++;
  actionCount++;

  ledState = !ledState;
  digitalWrite(LED_PIN, ledState);

  Serial.println();
  Serial.println("================================");
  Serial.printf("LOGICAL EVENT #%lu\n", logicalEvents);
  Serial.println("Button press ACCEPTED");

  noInterrupts();
  uint32_t raw = rawInterrupts;
  interrupts();

  uint32_t rawSinceLastEvent = raw - lastLoggedRawInterrupts;
  lastLoggedRawInterrupts = raw;

  Serial.printf("RAW interrupts total: %lu\n", raw);
  Serial.printf("RAW interrupts since previous logical event: %lu\n", rawSinceLastEvent);
  Serial.printf("LOGICAL events total: %lu\n", logicalEvents);
  Serial.printf("ACTIONS total: %lu\n", actionCount);
  Serial.println("ACTION executed ONCE");
  Serial.println("LOGGER: physical press -> RAW -> LOGICAL -> ACTION");
  Serial.println("================================");

  // Simulate a critical non-blocking operation
  operationUntil = millis() + 2000;
}

void setup() {
  Serial.begin(115200);
  // Give Wokwi virtual UART time to attach before the first log lines.
  delay(500);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  stableState = digitalRead(BUTTON_PIN);

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    buttonISR,
    CHANGE
  );

  Serial.println();
  Serial.println("ESP32 BUTTON / INTERRUPT / DEBOUNCE DEMO");
  Serial.println("----------------------------------------");
  Serial.println("GPIO18 -> INTERRUPT -> ISR -> DEBOUNCE -> EVENT");
  Serial.println();
  Serial.println("Green BUTTON = physical button with bouncing");
  Serial.println("Red NOISE    = artificial noise input");
  Serial.println();
}

void loop() {
  if (edgePending) {
    uint32_t edgeTime;

    noInterrupts();
    edgeTime = lastEdgeUs;
    interrupts();

    // Wait until signal remains unchanged for DEBOUNCE_US
    if ((uint32_t)(micros() - edgeTime) >= DEBOUNCE_US) {
      int currentState = digitalRead(BUTTON_PIN);
      bool canProcess = false;

      noInterrupts();
      if (lastEdgeUs == edgeTime) {
        edgePending = false;
        canProcess = true;
      }
      interrupts();

      if (canProcess && currentState != stableState) {
        stableState = currentState;

        // INPUT_PULLUP:
        // LOW = pressed
        // HIGH = released
        if (stableState == LOW) {
          acceptButtonPress();
        } else {
          Serial.println("Button RELEASE accepted");
        }
      }
    }
  }

  // Non-blocking "critical operation"
  static bool operationMessageShown = false;

  if ((int32_t)(operationUntil - millis()) > 0) {
    if (!operationMessageShown) {
      Serial.println("[SYSTEM] Critical operation is running...");
      operationMessageShown = true;
    }
  } else {
    if (operationMessageShown) {
      Serial.println("[SYSTEM] Critical operation finished.");
      operationMessageShown = false;
    }
  }

  // Periodic statistics
  static uint32_t previousRaw = 0;
  static uint32_t previousPrint = 0;
  static uint32_t previousHeartbeat = 0;

  // Serial connectivity heartbeat: proves that Wokwi is receiving output.
  if (millis() - previousHeartbeat >= 2000) {
    previousHeartbeat = millis();
    Serial.println("[SERIAL] Wokwi UART alive");
  }

  if (millis() - previousPrint >= 3000) {
    previousPrint = millis();

    noInterrupts();
    uint32_t raw = rawInterrupts;
    interrupts();

    if (raw != previousRaw) {
      Serial.println();
      Serial.printf(
        "[LOGGER] RAW interrupts: %lu | Logical events: %lu | Actions: %lu\n",
        raw,
        logicalEvents,
        actionCount
      );
      previousRaw = raw;
    }
  }
}
