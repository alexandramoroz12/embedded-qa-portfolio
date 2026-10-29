# Запуск Wokwi_I2C_BUGGY у VS Code

## Одноразове налаштування
1. Встановіть розширення **PlatformIO IDE** і **Wokwi Simulator** (VS Code запропонує їх з `.vscode/extensions.json`).
2. Активуйте ліцензію Wokwi: `F1` → **Wokwi: Request a New License**.

## Запуск
1. `File → Open Folder…` → відкрийте **саме цю папку** (`Wokwi_I2C_BUGGY`), де лежать `platformio.ini`, `wokwi.toml`, `diagram.json`.
2. `F1` → **PlatformIO: Build**. Дочекайтеся `SUCCESS` (перша збірка завантажує toolchain ESP32, кілька хвилин).
3. `F1` → **Wokwi: Start Simulator** (або відкрийте `diagram.json` і натисніть ▶).
4. Вивід прошивки (`[DATA] Temperature …`) з'являється у вкладці терміналу Wokwi під схемою.
5. FAULT перемикається кліком по слайд-перемикачу на схемі під час симуляції.

## Альтернатива: PlatformIO Serial Monitor
Поки симулятор працює, `F1` → **PlatformIO: Serial Monitor** підключається до `rfc2217://localhost:4000` (налаштовано в `platformio.ini`).

## Logic Analyzer
Після зупинки симуляції Wokwi зберігає VCD-файл у папку проєкту (у VS Code він може називатися `wokwi.vcd`, а не `i2c-capture.vcd`). Відкрийте його в PulseView через **Import Value Change Dump data**.

## Типові проблеми
- **"firmware not found"** — не виконано Build або відкрито не ту папку.
- **Немає виводу** — перезапустіть симулятор після Build; клацніть у вкладку терміналу Wokwi.
- **Шлях з кирилицею/пробілами** може ламати збірку на Windows — перенесіть проєкт, наприклад, у `C:\QA\Wokwi_I2C_BUGGY`.
