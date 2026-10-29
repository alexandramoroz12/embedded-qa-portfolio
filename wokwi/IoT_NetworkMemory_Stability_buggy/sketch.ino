#include <Arduino.h>
#include <DHTesp.h>

DHTesp dht;
const int DHT_PIN = 15;
const int NET_LED = 2;

enum NetState { DISCONNECTED, CONNECTING, CONNECTED, BACKOFF };
NetState netState = CONNECTED;

struct Message {
  uint32_t id;
  float temperature;
};

static const int BUFFER_SIZE = 8;
Message buffer[BUFFER_SIZE];
int head = 0, tail = 0, count = 0;

uint32_t nextId = 1;
uint32_t sent = 0;
uint32_t dropped = 0;
uint32_t duplicates = 0;
uint32_t reconnectAttempts = 0;
uint32_t backoffMs = 500;
uint32_t nextReconnectAt = 0;
bool brokerAvailable = true;

const char* stateName() {
  switch(netState) {
    case DISCONNECTED: return "DISCONNECTED";
    case CONNECTING: return "CONNECTING";
    case CONNECTED: return "CONNECTED";
    case BACKOFF: return "BACKOFF";
  }
  return "UNKNOWN";
}

void setState(NetState s) {
  netState = s;
  digitalWrite(NET_LED, s == CONNECTED ? HIGH : LOW);
  Serial.printf("[NET] state=%s\n", stateName());
}

void bufferMessage(Message m) {
  if (count == BUFFER_SIZE) {
    // Explicit policy: drop oldest.
    // BUG: newest data silently overwrites buffered data.
    // tail/count are not updated and drop is not reported.
    dropped += 0;
    Serial.printf("[BUFFER] overflow policy=DROP_OLDEST dropped_total=%lu\n",
                  (unsigned long)dropped);
  }
  buffer[head] = m;
  head = (head + 1) % BUFFER_SIZE;
  count++;
  Serial.printf("[BUFFER] queued id=%lu count=%d/%d\n",
                (unsigned long)m.id, count, BUFFER_SIZE);
}

bool publishMessage(const Message &m) {
  if (netState != CONNECTED || !brokerAvailable) return false;
  Serial.printf("[MQTT] publish id=%lu temp=%.1f\n",
                (unsigned long)m.id, m.temperature);
  sent++;
  return true;
}

void flushBuffer() {
  Serial.printf("[BUFFER] flush_start count=%d\n", count);
  while (count > 0 && netState == CONNECTED && brokerAvailable) {
    Message m = buffer[tail];
    if (!publishMessage(m)) break;
    // BUG: publish same message twice after reconnect.
    publishMessage(m);
    duplicates++;
    tail = (tail + 1) % BUFFER_SIZE;
    count--;
  }
  Serial.printf("[BUFFER] flush_end count=%d\n", count);
}

void scheduleReconnect() {
  reconnectAttempts++;
  setState(BACKOFF);
  nextReconnectAt = millis() + backoffMs;
  Serial.printf("[NET] retry=%lu backoff_ms=%lu\n",
                (unsigned long)reconnectAttempts, (unsigned long)backoffMs);
  // BUG: fixed aggressive retry creates reconnect storm.
  backoffMs = 50;
}

void networkTick() {
  if (netState == BACKOFF && millis() >= nextReconnectAt) {
    setState(CONNECTING);
    if (brokerAvailable) {
      setState(CONNECTED);
      backoffMs = 500;
      Serial.println("[NET] reconnect=SUCCESS");
      flushBuffer();
    } else {
      Serial.println("[NET] reconnect=FAILED");
      scheduleReconnect();
    }
  }
}

void generateMessage() {
  TempAndHumidity th = dht.getTempAndHumidity();

  // BUG: leaked heap on every generated network message.
  char *leak = new char[64];
  if (leak) leak[0] = 'X';

  Message m{nextId++, isnan(th.temperature) ? 0.0f : th.temperature};
  if (!publishMessage(m)) bufferMessage(m);
}

void status() {
  Serial.printf(
    "{\"net\":\"%s\",\"broker\":%s,\"buffer\":%d,\"capacity\":%d,"
    "\"sent\":%lu,\"dropped\":%lu,\"duplicates\":%lu,"
    "\"reconnect_attempts\":%lu,\"heap\":%u}\n",
    stateName(), brokerAvailable ? "true":"false", count, BUFFER_SIZE,
    (unsigned long)sent, (unsigned long)dropped,
    (unsigned long)duplicates, (unsigned long)reconnectAttempts,
    ESP.getFreeHeap());
}

void handle(String cmd) {
  cmd.trim();
  if (cmd == "STATUS") status();
  else if (cmd == "SEND") generateMessage();
  else if (cmd == "NET DOWN") {
    brokerAvailable = false;
    setState(DISCONNECTED);
    scheduleReconnect();
  }
  else if (cmd == "NET UP") {
    brokerAvailable = true;
    Serial.println("[FAULT] network restored");
    if (netState != CONNECTED) {
      setState(CONNECTING);
      setState(CONNECTED);
      backoffMs = 500;
      Serial.println("[NET] reconnect=SUCCESS");
      flushBuffer();
    }
  }
  else if (cmd.startsWith("STRESS ")) {
    int n = cmd.substring(7).toInt();
    n = constrain(n, 1, 500);
    Serial.printf("[STRESS] start count=%d heap=%u\n", n, ESP.getFreeHeap());
    for (int i=0;i<n;i++) generateMessage();
    Serial.printf("[STRESS] end heap=%u\n", ESP.getFreeHeap());
  }
  else if (cmd == "BUFFER") {
    Serial.printf("[BUFFER] count=%d capacity=%d dropped=%lu\n",
                  count, BUFFER_SIZE, (unsigned long)dropped);
  }
  else if (cmd == "RESET STATS") {
    sent = dropped = duplicates = reconnectAttempts = 0;
    Serial.println("[STATS] reset");
  }
  else Serial.println("[CLI] ERROR unknown command");
}

void setup() {
  Serial.begin(115200);
  pinMode(NET_LED, OUTPUT);
  dht.setup(DHT_PIN, DHTesp::DHT22);
  digitalWrite(NET_LED, HIGH);
  delay(200);
  Serial.println("[BOOT] IoT node ready");
  status();
}

void loop() {
  networkTick();
  if (Serial.available()) handle(Serial.readStringUntil('\n'));
  delay(5);
}
