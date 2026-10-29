# Smart Gate BUGGY — VS Code + PlatformIO + Wokwi

Навмисно багована версія для QA-практики. Не виправляйте дефекти перед тестуванням.

## Запуск
1. Відкрити цю папку у VS Code.
2. Дочекатися встановлення PlatformIO.
3. PlatformIO: Build або `pio run`.
4. Після SUCCESS: F1 → `Wokwi: Start Simulator`.

## Структура
- `src/main.cpp` — BUGGY firmware
- `diagram.json` — Wokwi схема
- `platformio.ini` — Arduino Uno / PlatformIO
- `wokwi.toml` — шляхи до firmware
- `REQUIREMENTS.md` — вимоги для тестування
