# Homework — GPIO Interrupt & Button Debounce QA

Навчальний **buggy build** для VS Code + PlatformIO + Wokwi.

## Запуск

1. Відкрийте **саме корінь цієї папки** у VS Code.
2. Виконайте `PlatformIO: Build`.
3. Запустіть `Wokwi: Start Simulator`.
4. Для цього завдання використовуйте **Serial Monitor / Terminal усередині Wokwi**, а не `PlatformIO: Monitor`.

## Виправлений Serial Monitor

У `diagram.json` явно підключено UART0 ESP32 до Wokwi Serial Monitor:

- `esp:TX0` → `$serialMonitor:RX`
- `esp:RX0` → `$serialMonitor:TX`

Serial Monitor налаштований як `display: always`.

У `setup()` є затримка 500 мс після `Serial.begin(115200)`, щоб віртуальний UART встиг підключитися до першого виводу.

Додатково firmware кожні 2 секунди друкує:

```text
[SERIAL] Wokwi UART alive
```

Це спеціальний heartbeat для перевірки самого каналу Serial.

## Якщо Terminal знову порожній

Перевірте:

1. `PlatformIO: Build` → **SUCCESS**.
2. Відкрита саме ця папка проєкту.
3. Відкритий `diagram.json` саме з цієї папки.
4. `F1 → Wokwi: Start Simulator`.
5. Вікно симулятора залишається відкритим/видимим.
6. Відкрити **Serial Monitor / Terminal у Wokwi**.

Очікувано через максимум 2 секунди має з'явитися:

```text
[SERIAL] Wokwi UART alive
```

А після старту:

```text
ESP32 BUTTON / INTERRUPT / DEBOUNCE DEMO
GPIO18 -> INTERRUPT -> ISR -> DEBOUNCE -> EVENT
```

Wokwi зазначає, що для VS Code симуляція повинна залишатися видимою, інакше вона може призупинитися та serial output не надходитиме.

## Стенд

- ESP32 DevKit v1
- `BUTTON` → GPIO18, фізична кнопка з bouncing
- `NOISE` → GPIO18, штучні короткі зміни сигналу
- `LOGICAL EVENT` LED → GPIO2
- Logic Analyzer:
  - `D0` — RAW GPIO18
  - `D1` — GPIO2 / прийнята логічна дія

## Logger

У firmware вбудований Serial Logger:

**Physical action → RAW interrupts → LOGICAL events → ACTIONS**

Приклад:

```text
LOGICAL EVENT #1
Button press ACCEPTED
RAW interrupts total: 4
RAW interrupts since previous logical event: 4
LOGICAL events total: 1
ACTIONS total: 1
ACTION executed ONCE
LOGGER: physical press -> RAW -> LOGICAL -> ACTION
```

## Основна вимога

**Одне фізичне натискання має створювати рівно одну логічну press-подію та одну action.**

Нестабільні переходи RAW-сигналу та додаткові ISR самі по собі не є дефектом. Дефект виникає, якщо вони породжують зайві logical events/actions або якщо валідна дія губиться.

## Що здати

- Hardware Test Sheet / Test Report;
- Logger evidence table;
- щонайменше 1 підтверджений Bug Report;
- evidence: Serial Logger + Logic Analyzer capture;
- короткий QA Conclusion.

Деталі сценаріїв — у `HOMEWORK.md` та `test_cases.md`.
