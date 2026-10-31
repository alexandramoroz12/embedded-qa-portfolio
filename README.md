# Embedded IoT Controller (ESP32) — Embedded QA Case Study

## What this repository demonstrates
How seven separate practical assignments from the Embedded QA course turn into one evidence-based release decision for embedded firmware: risk-based testing, fault injection, network stress, memory stability and test automation. The conclusion is **NO-GO**, and every point is backed by evidence that anyone can re-check.

## System
A reference IoT controller based on the ESP32: it accepts commands over UART/CLI, reads sensors (DHT22, I2C), drives a relay, PWM and an LED, and sends telemetry over Wi-Fi/MQTT. It is a composite device: its subsystems were tested on different Wokwi set-ups (the mapping is in section 1 of [`MASTER_TEST_PLAN.md`](MASTER_TEST_PLAN.md)).

```
 INPUTS                       ESP32 FIRMWARE                      OUTPUTS
 CLI (UART) ──────────┐                                     ┌── Relay / PWM / LED
 Sensors (DHT22, I2C) ─┼──►  parser · state machine ·  ──────┼── MQTT telemetry
 Button / limit switch ┤      WDT · offline buffer · NVS     └── Serial logs / STATUS
 Network (Wi-Fi / MQTT)┘
```

## Main quality risks
- **Actuators left in an unsafe state** after a fault or reset (relay, PWM, LED).
- **Data loss and corruption:** silent offline-buffer overflow, duplicates, memory leak.
- **Peripheral failure without a controlled state** (I2C, DHT22) and firmware hangs.
- **Faulty command handling** and false input events (CLI, bounce/noise).

## Test strategy
Risks are turned into levels, methods and environments in [`MASTER_TEST_PLAN.md`](MASTER_TEST_PLAN.md) and [`FINAL_TEST_MATRIX.md`](FINAL_TEST_MATRIX.md).

## Key evidence
Full list: [`EVIDENCE_INDEX.md`](EVIDENCE_INDEX.md). The strongest items:
- `bug-reports/WDT_hang.mp4`: hang → WDT reset in ≈3 s by the Wokwi timer, but the relay stays ON (E04).
- `network-stress/HW7_IoT_Network_Test_Report_EN.pdf`: buffer overflow with `dropped=0` and a heap trend of −23% (E07, E08).
- `pulseview/HW4_I2C_Test_Report_EN.pdf`: NACK → the main loop stops (E03).

## Key findings
24 defects across 7 practices (12 Critical, 2 High, 8 Major, 2 Low/Medium). The most important:
- the relay stays ON after a WDT reset, and `RESET` leaves PWM=255 and the LED ON;
- with more than 8 offline messages, some IDs are lost while `dropped=0`, so the loss is silent;
- a memory leak of ≈80 bytes per message: ≈23% of heap over 1000 messages;
- SYSTEM FAULT on input of 41+ characters and on `set` while the configuration is DIRTY;
- after the I2C device is lost, the main loop stops and does not recover without a restart.

All of them hit safety and data-integrity requirements, so they block the release.

## Automation
Pytest automation of the ESP32 firmware through Wokwi (24 tests: 20 passed, 4 failed). Code: `pytest/`. Original repository: https://github.com/alexandramoroz12/embeded_firmware-autotests_hw6.

How to run (the Wokwi simulation must be running in VS Code):
```
set DEVICE_PORT=rfc2217://localhost:4000
python -m pytest tests -v
```
Windows 11, Python 3.14.7, pytest 9.1.1, pyserial 3.5.

## Limitations
Everything was tested in simulation on BUGGY builds (black-box). The following need real hardware or a real network: power loss during `save`, deep-sleep current, physical bounce and noise, a real Wi-Fi/MQTT broker, long soak tests. Not covered: SPI, security, OTA, BLE. Details: section 16 of `MASTER_TEST_PLAN.md`.

## Conclusion
**NO-GO.** Critical safety and data-integrity defects are open, and the critical regression does not pass. The team can move to GO WITH RISKS after all Critical defects are fixed and retested with new evidence. Rationale and conditions: [`RELEASE_DECISION.md`](RELEASE_DECISION.md). The engineering story: [`CASE_STUDY.md`](CASE_STUDY.md).

## Repository structure

| Path | Contents |
|---|---|
| `README.md` | This file |
| `MASTER_TEST_PLAN.md` | Test plan, risks, criteria |
| `FINAL_TEST_MATRIX.md` | Matrix: risk → test → evidence → status |
| `EVIDENCE_INDEX.md` | Index of evidence |
| `CASE_STUDY.md` | The engineering story of the project |
| `RELEASE_DECISION.md` | GO / NO-GO decision |
| `bug-reports/` | Bug reports per topic (HW1–HW7), with screenshots |
| `network-stress/` | HW7 report and Serial Monitor logs (network and memory stability) |
| `pulseview/` | HW4 report (I2C protocol analysis in PulseView) |
| `pytest/` | Python/pytest autotest code |
| `test-reports/` | Full test reports for HW1, HW2, HW3, HW5 and HW6 |
| `wokwi/` | Wokwi projects from VS Code |
| `resume/` | Resume |
