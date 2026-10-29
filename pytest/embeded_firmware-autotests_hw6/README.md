# Firmware autotests (Pytest + Wokwi)

Автотести для прошивки ESP32, що працює в симуляторі Wokwi. Python
спілкується з пристроєм через Serial/RFC2217 (`pyserial`), відповіді
пристрою це JSON-рядки.

## Структура

- `device.py` - клас `Device`: відправка команд і розбір JSON-відповіді
- `conftest.py` - фікстура `device` (precondition `SET LED OFF` перед тестом, cleanup після)
- `tests/` - тести (статус, реле/LED, датчик, межові значення, timeout)
- `check.py` - допоміжний скрипт для діагностики зв'язку та часу відповіді
- `pytest.ini`, `requirements.txt` - налаштування й залежності

## Запуск

1. Запустити симуляцію Wokwi (RFC2217 на порту 4000).
2. Встановити залежності:

```
pip install -r requirements.txt
```

3. Вказати порт і запустити тести (PowerShell):

```
$env:DEVICE_PORT="rfc2217://localhost:4000"
python -m pytest -v
```

Для реальної плати замість URL вказують порт, наприклад `COM5`.

## Що покрито

- базові команди: `PING`, `STATUS`, `GET TEMP`, `SET LED ON/OFF`, `SET PWM N`, `RESET`
- межові та негативні значення `SET PWM`, невідомі команди
- поведінка при timeout (`NO_RESPONSE` -> `TimeoutError`)
- контрольований precondition і cleanup

## Відомі FAIL і їх класифікація

| Тест | Симптом | Категорія |
|---|---|---|
| `test_ping` | `PING` відповідає за ~1,3 с, клієнтський timeout 1,0 с | Communication (вимоги до часу відповіді немає) |
| `test_pwm_valid_boundaries[100]` | `SET PWM 100` -> `ERROR` (вимога 5: 0..100 включно) | Firmware |
| `test_reset_restores_safe_state` | після `RESET` `pwm` лишається 255 (вимога 7) | Firmware |
| `test_temperature_is_valid_json` | відповідь `GET TEMP` без закривної `}` (вимога 3) | Firmware |
