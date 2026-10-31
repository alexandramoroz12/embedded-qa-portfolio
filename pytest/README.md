# Firmware autotests (Pytest + Wokwi)

Autotests for ESP32 firmware running in the Wokwi simulator. Python talks to the device over Serial/RFC2217 (pyserial); the device's responses are JSON strings.

## Structure

- `device.py` - the `Device`class: sends commands and parses the JSON response
- `conftest.py` - the`device` fixture (precondition `SET LED OFF` before each test, cleanup afterwards)
- `tests/` - tests (status, relay/LED, sensor, boundary values, timeout)
- `check.py` - helper script for diagnosing the connection and response time
- `pytest.ini`, `requirements.txt` - settings and dependencies

## How to run

1. Start the Wokwi simulation (RFC2217 on port 4000).
2. Install the dependencies:

```
pip install -r requirements.txt
```

3. Set the port and run the tests (PowerShell):

```
$env:DEVICE_PORT="rfc2217://localhost:4000"
python -m pytest -v
```

For a real board, specify a port such as`COM5`instead of the URL.

## What is covered

- basic commands: `PING`, `STATUS`, `GET TEMP`, `SET LED ON/OFF`, `SET PWM N`, `RESET`
- boundary and negative values of `SET PWM`, unknown commands
- timeout behaviour (`NO_RESPONSE` -> `TimeoutError`)
- controlled precondition and cleanup

## Known FAILs and their classification

| Test | 	Symptom | Category |
|---|---|---|
| `test_ping` | `PING` responds in ~1.3 s, the client timeout is 1.0 s | Communication (there is no response-time requirement) |
| `test_pwm_valid_boundaries[100]` | `SET PWM 100` -> `ERROR` (requirement 5: 0..100 inclusive) | Firmware |
| `test_reset_restores_safe_state` | after `RESET` `pwm` the pwm stays 255 (requirement 7) | Firmware |
| `test_temperature_is_valid_json` | the `GET TEMP` response has no closing `}` (requirement 3) | Firmware |
