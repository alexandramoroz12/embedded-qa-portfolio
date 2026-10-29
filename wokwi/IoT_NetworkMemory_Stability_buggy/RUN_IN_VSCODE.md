# Запуск симуляції у VS Code

## Одноразове налаштування
1. Встановіть розширення **PlatformIO IDE** і **Wokwi Simulator** (VS Code запропонує їх автоматично з `.vscode/extensions.json`).
2. Активуйте ліцензію Wokwi: `F1` → **Wokwi: Request a New License** (безкоштовно, потрібен акаунт Wokwi).

## Запуск
1. Відкрийте у VS Code **саме цю папку** (`File → Open Folder…`) — у корені мають бути `platformio.ini`, `wokwi.toml`, `diagram.json`.
2. `F1` → **PlatformIO: Build** (перша збірка завантажує toolchain ESP32, це кілька хвилин).
3. `F1` → **Wokwi: Start Simulator** (або відкрийте `diagram.json` і натисніть ▶).
4. Клацніть у вкладку терміналу Wokwi, наберіть команду (наприклад `STATUS`) і натисніть Enter. Набраний текст у терміналі Wokwi **не відображається** — це нормально.

Команди CLI: `STATUS`, `SEND`, `NET DOWN`, `NET UP`, `BUFFER`, `STRESS N`, `RESET STATS` (регістр важливий).

## Альтернатива: PlatformIO Serial Monitor (видно введення)
Поки симулятор запущено, він відкриває порт `rfc2217://localhost:4000`.
`F1` → **PlatformIO: Serial Monitor** підключиться до нього автоматично (`monitor_echo = yes`).
Для мітки часу біля кожного рядка (корисно для вимірювання backoff) додайте в `platformio.ini`: `monitor_filters = time`.

## Схема
DHT22 → GPIO15 (`esp:D15`), NET LED → GPIO2 (`esp:D2`). LED світиться, коли стан мережі CONNECTED.

## Типові проблеми
- **"firmware not found"** — не виконано Build, або відкрито батьківську папку.
- **Немає виводу / не приймає ввід** — симулятор на паузі; перезапустіть після нового Build.
- **Serial Monitor не підключається** — спочатку запустіть симулятор, потім монітор.
- **Відповідь на команду з'являється із затримкою ~1 с** — термінал надсилає лише `\r` без `\n`; використовуйте PlatformIO Serial Monitor (`monitor_eol = LF`).
