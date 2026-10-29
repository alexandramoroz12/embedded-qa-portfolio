#include <Arduino.h>
#include <DHTesp.h>
#include <Preferences.h>
#include <esp_sleep.h>
#include <esp_system.h>
#include <esp_task_wdt.h>
#include <esp_arduino_version.h>

static const int PIN_DHT = 15;
static const int PIN_BUTTON = 4;
static const int PIN_LED = 2;
static const int PIN_RELAY = 5;

static const uint32_t WDT_TIMEOUT_MS = 4000;

DHTesp dht;
Preferences prefs;

uint32_t eventCounter = 0;
bool relayState = false;
bool hangRequested = false;

const char* resetName(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON: return "POWER_ON";
    case ESP_RST_SW: return "SOFTWARE_RESET";
    case ESP_RST_DEEPSLEEP: return "DEEP_SLEEP";
    case ESP_RST_BROWNOUT: return "BROWNOUT";
    case ESP_RST_INT_WDT: return "INT_WDT";
    case ESP_RST_TASK_WDT: return "TASK_WDT";
    case ESP_RST_WDT: return "WDT";
    default: return "OTHER";
  }
}

void setupWdt() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  esp_task_wdt_config_t cfg = {
    .timeout_ms = WDT_TIMEOUT_MS,
    .idle_core_mask = (1U << portNUM_PROCESSORS) - 1,
    .trigger_panic = true
  };
  esp_task_wdt_init(&cfg);
#else
  esp_task_wdt_init(WDT_TIMEOUT_MS / 1000, true);
#endif
  esp_task_wdt_add(NULL);
  Serial.printf("[WDT] enabled timeout=%lu ms\n", (unsigned long)WDT_TIMEOUT_MS);
}

void applyRelay(bool on) {
  relayState = on;
  digitalWrite(PIN_RELAY, on ? HIGH : LOW);
  digitalWrite(PIN_LED, on ? HIGH : LOW);
}

void saveState() {
  prefs.putUInt("counter", eventCounter);
  delay(25);
  prefs.putBool("relay", relayState);
  Serial.println("[STORE] saved");
}

void printSensor() {
  TempAndHumidity th = dht.getTempAndHumidity();

  // Intentional defect: invalid sensor data is masked.
  if (isnan(th.temperature) || isnan(th.humidity)) {
    th.temperature = 0;
    th.humidity = 0;
  }

  Serial.printf("[SENSOR] temp=%.1fC humidity=%.1f%%\n",
                th.temperature, th.humidity);
}

void printStatus() {
  TempAndHumidity th = dht.getTempAndHumidity();
  bool sensorOk = !isnan(th.temperature) && !isnan(th.humidity);

  Serial.print("{\"state\":\"ACTIVE\",\"reset_reason\":\"");
  Serial.print(resetName(esp_reset_reason()));
  Serial.print("\",\"relay\":");
  Serial.print(relayState ? "true" : "false");
  Serial.print(",\"counter\":");
  Serial.print(eventCounter);
  Serial.print(",\"heap\":");
  Serial.print(ESP.getFreeHeap());
  Serial.print(",\"sensor_ok\":");
  Serial.print(sensorOk ? "true" : "false");
  Serial.println("}");
}

void buggySleep(uint32_t seconds) {
  if (seconds < 1) seconds = 1;
  if (seconds > 60) seconds = 60;

  eventCounter++;
  saveState();

  Serial.printf("[STATE] SLEEP timer=%lu s\n", (unsigned long)seconds);
  Serial.flush();
  delay(20);

  // Intentional defect: wrong microsecond conversion (10x too short).
  esp_sleep_enable_timer_wakeup((uint64_t)seconds * 100000ULL);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);
  esp_deep_sleep_start();
}

void simulateInterruptedSave() {
  eventCounter += 100;

  // Intentional defect: partial state is committed before reboot.
  prefs.putUInt("counter", eventCounter);

  Serial.println("[FAULT] power loss DURING save");
  Serial.flush();
  delay(20);
  esp_restart();
}

void printHelp() {
  Serial.println("Commands:");
  Serial.println("  help");
  Serial.println("  status");
  Serial.println("  sensor");
  Serial.println("  relay on");
  Serial.println("  relay off");
  Serial.println("  save");
  Serial.println("  sleep <seconds>");
  Serial.println("  hang");
  Serial.println("  powerloss");
  Serial.println("  reboot");
}

void handleCommand(String cmd) {
  cmd.trim();
  if (!cmd.length()) return;

  if (cmd == "help") {
    printHelp();
  } else if (cmd == "status") {
    printStatus();
  } else if (cmd == "sensor") {
    printSensor();
  } else if (cmd == "relay on") {
    applyRelay(true);
    eventCounter++;
    saveState();
    Serial.println("[RELAY] ON");
  } else if (cmd == "relay off") {
    applyRelay(false);
    eventCounter++;
    saveState();
    Serial.println("[RELAY] OFF");
  } else if (cmd == "save") {
    eventCounter++;
    saveState();
  } else if (cmd.startsWith("sleep")) {
    int p = cmd.indexOf(' ');
    uint32_t sec = 5;
    if (p > 0) sec = (uint32_t)cmd.substring(p + 1).toInt();
    buggySleep(sec);
  } else if (cmd == "hang") {
    Serial.println("[FAULT] firmware hang");
    Serial.flush();
    hangRequested = true;
  } else if (cmd == "powerloss") {
    simulateInterruptedSave();
  } else if (cmd == "reboot") {
    esp_restart();
  } else {
    Serial.println("[CLI] ERROR unknown command");
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  dht.setup(PIN_DHT, DHTesp::DHT22);

  prefs.begin("qa-lab", false);

  // Intentional defect: no integrity/version validation.
  eventCounter = prefs.getUInt("counter", 0);
  relayState = prefs.getBool("relay", false);

  Serial.printf("[BOOT] reset_reason=%s\n", resetName(esp_reset_reason()));

  // Intentional defect: unsafe output is restored after abnormal reset.
  applyRelay(relayState);

  esp_sleep_wakeup_cause_t wake = esp_sleep_get_wakeup_cause();
  if (wake != ESP_SLEEP_WAKEUP_UNDEFINED) {
    Serial.printf("[WAKE] cause=%d\n", (int)wake);
  }

  setupWdt();
  printHelp();
}

void loop() {
  if (hangRequested) {
    while (true) {
      // wait for watchdog
    }
  }

  esp_task_wdt_reset();

  static String line;
  while (Serial.available()) {
    char c = (char)Serial.read();
    // Enter: accept CR, LF or CRLF (Wokwi terminal sends CR)
    static bool prevWasCR = false;
    if (c == '\n' && prevWasCR) { prevWasCR = false; continue; }
    prevWasCR = (c == '\r');
    if (c == '\r') c = '\n';
    if (c == '\n') {
      handleCommand(line);
      line = "";
    } else if (c != '\r') {
      line += c;
    }
  }

  // Intentional defect: no debounce.
  static bool lastButton = HIGH;
  bool now = digitalRead(PIN_BUTTON);
  if (lastButton == HIGH && now == LOW) {
    applyRelay(!relayState);
    eventCounter++;
    saveState();
    Serial.printf("[BUTTON] relay=%s\n", relayState ? "ON" : "OFF");
  }
  lastButton = now;

  delay(1);
}
