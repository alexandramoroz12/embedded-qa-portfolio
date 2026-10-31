# Master Test Plan — Embedded IoT Controller (ESP32)

| | |
|---|---|
| **Author** | Alexandra Holoborodko |
| **Date** | 31.10.2026 |
| **Status** | Approved |
| **Builds** | BUGGY firmware (black-box, source code not inspected) |
| **Basis** | 7 practical assignments of the Embedded QA in IoT and DefTech course (HW1–HW7) |

---

## 1. Document Purpose

**Product.** A reference IoT controller based on the ESP32: it accepts commands over UART/CLI, reads sensors (DHT22, I2C), drives actuators (relay, PWM, LED) and sends telemetry over Wi-Fi/MQTT.

**Decision this plan supports:** can the firmware be released (**GO / GO WITH RISKS / NO-GO**).

The document links the product, quality risks, test methods, evidence and readiness criteria. The decision is made by the product owner and the test manager based on the evidence (section 14) and the exit criteria (section 12).

> **About the system under test.** This is a composite reference device. Its subsystems were tested on different Wokwi set-ups during the course. How subsystems map to practices:

| # | Practice | Set-up | What was tested | Result |
|---|---|---|---|---|
| HW1 | Black-box + Hardware Test Sheet | Smart Gate Controller, Arduino Uno, Wokwi | Gate states, limit switches, relays, timeout, ERROR | 27 TC: 23 passed, 3 failed, 1 blocked; 3 defects (1 Critical, 2 High); **Rejected** |
| HW2 | Debounce / GPIO | ESP32 DevKit v1, BUTTON + NOISE on GPIO18, LED on GPIO2 | RAW interrupt → logical event → action | TC-01…04 passed, TC-05/06 failed; 2 Critical |
| HW3 | UART / CLI | UART CLI Homework Device v2.0 | `speed`/`timeout` bounds, input length, parser, states | 19 TC; 7 defects (2 Critical, 5 Major); **not approved** |
| HW4 | I2C protocol | ESP32 + MPU6050 (0x68), SDA 21 / SCL 22, PulseView | ACK/NACK, peripheral loss, recovery | 1 Critical |
| HW5 | Recovery / Fault injection | ESP32 DevKit V1: DHT22 GPIO15, button GPIO4, LED GPIO2, relay GPIO5 | Sleep, WDT, power loss, sensor failure, repeated recovery | 3 defects (2 Critical, 1 Major) after one defect was withdrawn; **NO-GO** |
| HW6 | Python / pytest | ESP32 firmware in Wokwi, `rfc2217://localhost:4000` | CLI/JSON/relay autotests | 24 tests: 20 passed, 4 failed; 4 defects (2 Critical, 2 Low/Medium) |
| HW7 | IoT Network / Memory Stability | `homework_buggy`, Wokwi + PlatformIO | Network state, backoff, buffer, duplicates, heap | R1–R8: 4 requirements failed (R2, R4, R6, R7); 4 defects (2 Critical, 2 Major) |

In total **24 defects** were found (register in appendix A). Defect HW5 Bug_2 (WDT ≈30 s) was withdrawn after mentor feedback: 30 s was wall-clock time because of the simulator slowdown; by the Wokwi timer the reset took ≈3 s.

---

## 2. System Overview

```
 INPUTS                       ESP32 FIRMWARE                      OUTPUTS
 CLI (UART) ──────────┐                                     ┌── Relay / PWM / LED
 Sensors (DHT22, I2C) ─┼──►  parser · state machine ·  ──────┼── MQTT telemetry
 Button / limit switch ┤      WDT · offline buffer · NVS     └── Serial logs / STATUS
 Network (Wi-Fi / MQTT)┘            ▲
                            power / sleep / flash
```

- **Hardware:** ESP32 DevKit, DHT22, MPU6050 (I2C), button, LED, relay.
- **Firmware:** CLI parser, device state machine, watchdog (WDT), deep sleep, configuration storage, offline buffer (capacity 8, DROP_OLDEST policy), reconnect with backoff.
- **Interfaces:** GPIO, UART (115200), I2C, Wi-Fi, MQTT.
- **External dependencies:** power supply, MQTT broker, network, Wokwi simulator.
- **Critical data / actuators:** relay, PWM, LED; configuration (`speed`, `timeout`, `counter`); telemetry (unique message IDs); free heap.

**Critical states**

| Subsystem | States |
|---|---|
| Device | BOOT → ACTIVE → SLEEP; ERROR / FAULT; restart after WDT |
| Network | DISCONNECTED → CONNECTING → BACKOFF → CONNECTED |
| Gate (HW1) | CLOSED, OPENING, OPEN, CLOSING, STOPPED, ERROR |

---

## 3. Quality Objectives

| Characteristic | What we consider acceptable |
|---|---|
| **Safety** | After a fault/reset the actuators are in a safe state (relay OFF, PWM 0, LED OFF) |
| **Recoverability** | The device recovers from sensor loss, network loss and a WDT reset without a manual restart |
| **Robustness** | Invalid input does not cause a reboot/hang; always a controlled `ERROR` |
| **Data integrity** | No silent loss or duplication of messages; valid JSON |
| **Resource stability** | Free heap does not degrade monotonically |
| **Timing** | Sleep, WDT, command responses and reconnect are within the requirements |
| **Observability** | `STATUS` and logs reflect the real state (including the `dropped` counter) |

---

## 4. Scope

- **Firmware:** state logic, timers, event handling, configuration.
- **Interfaces:** UART/CLI, I2C, GPIO, Wi-Fi/MQTT (at the firmware-behaviour level).
- **Protocols:** response format (JSON), I2C ACK/NACK, message exchange and errors.
- **Scenarios:** functional, boundary, negative, command sequences.
- **Critical states:** recovery after faults, WDT, connection loss, power loss, sensor failure.
- **Non-functional:** memory stability, network stress, timing.

## 5. Out of Scope

| Not tested | Why |
|---|---|
| Circuit design, soldering, hardware defects | Hardware is outside firmware responsibility |
| Mechanics (enclosure, mounting, vibration) | Not verifiable in simulation |
| UI / design, mobile and cloud applications | Only if they are not a firmware requirement |
| MQTT broker implementation and third-party services | Not controlled by the team |
| SPI exchange | Only I2C was covered in the course |
| Security (authentication, OTA, encryption) | No requirements in the practices; moved to residual risks |

---

## 6. Assumptions / Constraints

- **Time is measured by the Wokwi timer or by log timestamps, not by the wall clock.** Wokwi can slow down to ≈2% speed (especially after `hang`), so real time is not device time.
- Testing is **black-box**: conclusions are drawn from observable behaviour, logs, `STATUS` and traces.
- **Wokwi** simulates logic but not the full physics: real contact bounce, electrical noise, power consumption, exact timing.
- The network is modelled with `NET DOWN` / `NET UP`. In the simulator a failed reconnect takes ≈45 ms; on real Wi-Fi it takes seconds.
- Logic analysis is done on a `.vcd` capture in PulseView, not on a physical analyzer.
- Requirements come from the practice assignments (R1…R16 in each). There is no separate requirements document for the whole device.
- Results apply to the specific **BUGGY builds**. A retest is needed after fixes.
- Autotests run through RFC2217 to the Wokwi simulation.

---

## 7. Quality Risks

Priority = Probability × Impact (scale 1–3): **7–9 High**, **4–6 Medium**, **1–3 Low**. Probability takes into account the facts already observed. Risk IDs match `FINAL_TEST_MATRIX.md`.

| Risk ID | Risk | Impact | Probability | Priority | Mitigation / Test |
|---|---|---|---|---|---|
| R01 | Button bounce / noise creates a false action | 3 | 2 | **6** | RAW → LOGICAL → ACTION in logs and Logic Analyzer (HW2). **Fact:** NOISE creates a false logical event and action, also during a critical operation |
| R02 | UART command parsing fails: out-of-range values, long input, parser errors | 3 | 3 | **9** | BVA + negative + length/state-sequence tests, pytest (HW3, HW6). **Fact:** input ≥41 characters and `set` in DIRTY → SYSTEM FAULT; `set speed 256` / `set timeout 0` accepted; `SET PWM 100` rejected |
| R03 | I2C/SPI transaction corrupted or peripheral lost without a controlled state | 3 | 3 | **9** | Fault injection + protocol trace in PulseView (HW4). **Fact:** NACK → `Error -1`, main loop stops, recovery only after a restart. SPI not covered |
| R04 | Firmware hangs and the WDT does not recover the device in time | 3 | 2 | **6** | `hang` test, repeated recovery ×10 (HW5). **Fact:** by the Wokwi timer `[FAULT]` → reboot ≈3 s (within the requirement); 10 of 10 recoveries stable |
| R05 | Power loss during a save corrupts persistent state | 3 | 2 | **6** | `powerloss` during `save` (HW5). **Fact:** `counter` changed, `relay` did not; no conclusion, real hardware needed |
| R06 | Network unavailable/unstable: reconnect storm, duplicated data | 3 | 3 | **9** | Scenario A (loss/recovery), B (offline buffer) (HW7). **Fact:** backoff 500→50 ms (≈20 attempts/s); every message in the flush is sent twice |
| R07 | Buffer overflow loses data silently | 3 | 3 | **9** | Scenario C (buffer limit 8/9/12) (HW7). **Fact:** with >8 messages some IDs are lost, `dropped=0` |
| R08 | Memory degradation (heap leak) | 3 | 3 | **9** | Scenario D: `STRESS 100` ×10 + heap trend (HW7). **Fact:** ≈80 bytes per message, −80268 bytes (≈23%) over 1000 messages |
| R09 | Gate controller enters an unsafe or undefined state | 3 | 3 | **9** | Black-box state machine, limit switches, timeout (HW1). **Fact:** state does not become OPEN after `LIMIT_OPEN`; no ERROR after >10 s in OPENING or with both limits active |
| R10 | Actuators left in an unsafe state after reset/fault (relay, PWM, LED) | 3 | 3 | **9** | Relay + WDT, `RESET`, ERROR logic (HW5, HW6). **Fact:** relay stays ON after the WDT; `RESET` leaves PWM=255 and LED ON |
| R11 | Sensor failure reported as valid data | 3 | 3 | **9** | Disconnect DHT22 (HW5). **Fact:** `[SENSOR] temp=0.0C humidity=0.0%` returned as a valid measurement |
| R12 | Timing and response-format deviations | 2 | 3 | **6** | Timing from timestamps (HW5), JSON validation and latency (HW6). **Fact:** `sleep 5` wakes in <1 s; `GET TEMP` without the closing `}`; `PING` ≈1.3 s against ≈0.17 s (classified as communication) |

---

## 8. Test Levels

| Level | What we verify | Risks | Evidence from the course |
|---|---|---|---|
| Component / firmware function | Parser, value bounds, response format | R02, R12 | HW3, HW6 |
| Interface / protocol | I2C ACK/NACK, UART JSON, command response | R03, R12 | HW4 (PulseView), HW6 |
| System (black-box) | Device states, working scenarios, gate state machine, sensor behaviour | R01, R09, R10, R11 | HW1, HW2, HW5 |
| Recovery / fault injection | `hang`, `powerloss`, sensor disconnect, FAULT ON/OFF, NET DOWN/UP | R03, R04, R05, R06, R10, R11 | HW4, HW5, HW7 |
| Network / reliability | Reconnect, offline buffer, buffer limit, memory soak | R06, R07, R08 | HW7 |
| Automation / regression | Repeatable deterministic checks | R02, R10, R12 | HW6 |

## 9. Test Types

| Type | Example from the practices |
|---|---|
| Functional | Valid CLI commands, OPEN/CLOSE/STOP of the gate |
| Boundary / negative | `speed` 0..255, `timeout` 1..60, input length 40/41, PWM 0..100 |
| Protocol | I2C ACK/NACK, JSON structure |
| Reliability | Repeated recovery after `hang` ×10 |
| Recovery | WDT, power loss, I2C FAULT OFF, NET UP |
| Persistence | `save` → `reset` → `status`; `powerloss` during `save` |
| Network stress | `SEND` ×N, buffer limit (8, 9, 12), `STRESS 100` |
| Memory stability | Memory Stability Log: heap after each iteration |
| Automation | 24 pytest tests (starter + own): 20 passed, 4 failed |

---

## 10. Test Environment

| Environment | Purpose | Tools |
|---|---|---|
| Wokwi | Firmware and set-up simulation (ESP32 DevKit v1; Arduino Uno for HW1) | Wokwi in VS Code, Wokwi Terminal |
| Serial / logs | Observability, evidence | PlatformIO Serial Monitor (`monitor_filters = time, log2file`), 115200 |
| Logic analysis | Protocol evidence | Wokwi Logic Analyzer → `.vcd`, PulseView |
| Host automation | Regression | Python 3.14.7, pytest 9.1.1, pyserial 3.5; `DEVICE_PORT=rfc2217://localhost:4000` |
| Real hardware | Electrical behaviour, noise, power consumption | **Not used so far** (see section 16) |

OS: Windows 11. Autotest run: `python -m pytest tests -v`.

---

## 11. Entry Criteria

- A stable firmware build with a version (`BUGGY` / `homework_buggy`) that loads in Wokwi.
- Requirements and expected behaviour are documented (the R-requirements of each practice).
- The test environment is ready: Wokwi, Serial Monitor, PulseView, Python environment.
- Known firmware and simulator limitations are recorded (section 6).
- Critical interfaces are observable: UART/CLI, I2C, network states, logs.

## 12. Exit Criteria

| # | Criterion | Status on 31.10.2026 |
|---|---|---|
| 1 | 100% of tests for risks with Priority ≥7 executed | Executed in simulation; not on real hardware |
| 2 | 0 open Blocker/Critical | **Not met:** 12 Critical + 2 High open |
| 3 | Regression passes 100% after fixes | **Not met:** 4 of 24 pytest tests failed; no retest |
| 4 | Recovery scenarios confirmed (WDT, I2C, network, sensor loss) | **Partial:** the network recovers in ≈2 ms, repeated recovery ×10 is stable, WDT reset ≈3 s; I2C, sensor-loss handling and the relay state after the WDT do not meet the requirements |
| 5 | Heap does not degrade monotonically; deviation of free heap after 1000 messages ≤1% of baseline *(proposed threshold)* | **Not met:** −23% |
| 6 | Evidence stored and available | **Met** (logs, traces, screenshots, pytest output, video) |
| 7 | Known risks accepted or documented | Documented in section 16; no formal acceptance yet |

---

## 13. Regression Strategy

**After every significant firmware change:**

| Level | Contents |
|---|---|
| **Smoke** | Boot without errors → basic CLI commands (`help`, `status`) → sensor read → network state CONNECTED in `STATUS` |
| **Critical regression** | Control + data integrity + recovery: `SET PWM 0/100`, `RESET` → safe state; `set speed/timeout` at the bounds; input of 40/41 characters; `set` in DIRTY; relay after `hang`; disconnected DHT22 → explicit error; I2C FAULT ON → OFF; Scenarios B and C (8/9/12) checking `sent`, `dropped`, `duplicates`; Scenario D (heap trend); ERROR on timeout and on two active limits (HW1) |
| **Full regression** | All functional, protocol, power/recovery, network and stress scenarios of HW1–HW7 |

**Automation.** We automate checks that are repeatable and deterministic:

- already automated: pytest (`test_boundaries`, `test_relay`, `test_sensor`, `test_status`, `test_timeout`, `test_my_cases`);
- next candidates: UART/CLI boundaries from HW3; HW7 network scenarios (all driven by `NET DOWN`, `SEND`, `BUFFER`, `STATUS`, `STRESS`); `Δheap` check after `STRESS`; WDT/sleep timing from log timestamps;
- mark known failures with `xfail(strict=True)` and the defect ID, so that a fix is noticed instead of "breaking" the run;
- PulseView analysis and checks that need a physical device stay manual.

---

## 14. Evidence Strategy

| Finding type | Evidence |
|---|---|
| CLI/logic failure | Serial log with timestamps + pytest output |
| I2C protocol | `.vcd` capture, PulseView (transaction screenshots) + `Wire.cpp` log |
| Recovery / WDT / sleep | Reset log (`reset_reason`), Wokwi timer timestamps, video |
| Network | `[NET]`, `[BUFFER]`, `[MQTT]` lines with time, `STATUS` JSON |
| Memory | Memory Stability Log: heap, `Δheap`, message count per iteration |
| Automation | `pytest` report, `check.py` (latency measurements) |
| Defect | Bug report: expected / actual, raw evidence, reproducibility |
| Summary | Test summary (TC count, passed/failed, defects by severity) |

Every conclusion must rest on an observed fact, not on an assumption about the code. The evidence index is in [`EVIDENCE_INDEX.md`](EVIDENCE_INDEX.md).

---

## 15. Defect Management

**Severity**

| Severity | Criterion | Example from the course |
|---|---|---|
| **Critical** | Unsafe actuator state, crash/reboot, silent data loss, memory exhaustion, no recovery | Relay ON after the WDT; SYSTEM FAULT on 41 characters; leak of ≈80 B/message |
| **High** | A safety protection is missing, but the actuator is not directly in an unsafe state | No ERROR after >10 s in OPENING; no ERROR on two active limits |
| **Major** | Wrong functional behaviour without direct safety impact, timing violation | `set speed 256` → OK; backoff 50 ms; duplicates |
| **Low / Medium** | Format or latency that breaks only some clients | `GET TEMP` without `}`; `PING` ≈1.3 s |

**Mandatory bug report fields:** Title, Requirement / Risk, Environment, Preconditions, Steps, Expected, Actual, Raw evidence, Severity, Reproducibility (N/N).

**Triage before filing a defect** (`FAILED` ≠ firmware bug): test logic → environment → communication → requirement → firmware. Example from HW6: `PING` fails because of the 1.0 s client timeout although the response is correct and arrives in ≈1.3 s. This is classified as communication, not a logic error.

**Measurement check.** Before filing a timing defect, time is verified against the Wokwi timer or log timestamps. Example from HW5: "WDT reset in ≈30 s" was withdrawn, because it was wall-clock time while Wokwi was running at ≈2% speed.

**Retest.** After a fix, the failed test and the critical regression are repeated. A defect is closed only with new evidence.

---

## 16. Residual Risks

Not verified, or verified only in simulation:

- **Real hardware:** deep-sleep consumption, wake-up reliability, physical bounce and noise, electrical parameters.
- **Power loss during `save`:** the HW5 result has no conclusion; on a real device we must check persistence of a non-empty buffer (it is in RAM) and configuration integrity.
- **Real Wi-Fi and MQTT broker:** the real reconnect attempt rate (seconds instead of ≈45 ms), QoS, broker behaviour on duplicates, drops during a flush.
- **Soak tests:** long runs (hours/days) with heap and watchdog monitoring; the threshold of ≈4400 messages before memory exhaustion is only an extrapolation from the simulation.
- **Autotests on real hardware** (currently Wokwi only).
- **Timing measured earlier by wall-clock:** `sleep 5` (HW5 Bug_1) and `PING` latency (HW6 Bug_4) should be re-checked against the Wokwi timer.
- **Not covered:** SPI, security, OTA, BLE.

---

## 17. Release Recommendation

### **NO-GO**

**Rationale.** The critical scenarios were executed, but safety and reliability requirements are violated and Exit Criteria 2–5 are not met. The tested builds show:

| Blocking risk | Defects |
|---|---|
| R10 Unsafe actuator state after a fault | relay ON after the WDT; `RESET` does not return PWM/LED to a safe state |
| R09 Gate state machine | no ERROR on timeout and on two active limits; state does not become OPEN |
| R03 / R11 Sensor and peripheral failure | I2C does not recover; `temp=0.0` reported as valid data |
| R02 Input robustness | SYSTEM FAULT on long input and on `set` in DIRTY |
| R07 / R08 Data integrity and memory | silent message loss (`dropped=0`); leak of ≈80 B/message |
| R01 False events | NOISE creates a logical event and action |

**Conditions for moving to GO WITH RISKS:**

1. All Critical defects are fixed and pass a retest with new evidence.
2. The critical regression passes at 100%.
3. Major defects are fixed or accepted as known risks by the product owner.
4. There is a plan to verify residual risks (section 16) on real hardware.

---

## Appendix A. Defect Register

| ID | Practice | Description | Severity |
|---|---|---|---|
| HW1-Bug_1 | HW1 | State does not become OPEN after `LIMIT_OPEN`; blocks TC-04 | Critical |
| HW1-Bug_2 | HW1 | No ERROR when OPENING lasts >10 s without a limit | High |
| HW1-Bug_3 | HW1 | No ERROR when both limits are active at the same time | High |
| HW2-Bug_1 | HW2 | NOISE creates a false logical event and action (TC-05) | Critical |
| HW2-Bug_2 | HW2 | The same during a critical operation (TC-06) | Critical |
| HW3-BUG-01 | HW3 | SYSTEM FAULT on input >40 characters (boundary: 41) | Critical |
| HW3-BUG-02 | HW3 | Invalid commands (`set speed5`, Cyrillic text) return `OK` | Major |
| HW3-BUG-03 | HW3 | `set speed 256` → `OK: speed=255` | Major |
| HW3-BUG-04 | HW3 | `set timeout 0` → `OK`, value changes | Major |
| HW3-BUG-05 | HW3 | Input of special characters only (`#@*&`) → `OK` | Major |
| HW3-BUG-06 | HW3 | Upper-case `STATUS` is accepted although commands are case-sensitive | Major |
| HW3-BUG-07 | HW3 | SYSTEM FAULT on `set` with `CONFIG=DIRTY` | Critical |
| HW4-Bug | HW4 | After the I2C device is lost (NACK, `Error -1`) exchange and the main loop stop; no recovery without a restart | Critical |
| HW5-Bug_1 | HW5 | `sleep 5` wakes in <1 s (expected 4.25–5.75 s) | Major |
| HW5-Bug_3 | HW5 | Relay stays ON after a WDT reset | Critical |
| HW5-Bug_4 | HW5 | Disconnected DHT22: `temp=0.0 humidity=0.0` as valid data | Critical |
| HW6-Bug_1 | HW6 | `SET PWM 100` → `ERROR INVALID_VALUE` | Critical |
| HW6-Bug_2 | HW6 | `RESET` leaves PWM=255 and LED ON | Critical |
| HW6-Bug_3 | HW6 | `GET TEMP` returns invalid JSON (no `}`) | Low/Medium |
| HW6-Bug_4 | HW6 | `PING` answers in ≈1.3 s; with a 1.0 s timeout the test fails (classification: communication) | Low/Medium |
| HW7-BUG-01 | HW7 | Backoff 500→50 ms, ≈20 attempts/s (reconnect storm) | Major |
| HW7-BUG-02 | HW7 | Every buffered message is published twice after NET UP | Major |
| HW7-BUG-03 | HW7 | Buffer overflow: count>8, `dropped=0`, IDs lost, flush corrupted | Critical |
| HW7-BUG-04 | HW7 | Memory leak ≈80 bytes per message, heap does not recover | Critical |

**Total: 24 defects**: 12 Critical, 2 High, 8 Major, 2 Low/Medium.

*Withdrawn: HW5-Bug_2 (WDT reset ≈30 s). By the simulation timer `[FAULT]` was at ≈11:52.4 and the reboot at ≈11:55.4, i.e. ≈3 s. The 30 s was wall-clock time, because after `hang` Wokwi slows down to ≈2% speed.*

## Appendix B. Where the Evidence Lives

| Artifact | Repository folder | Contents |
|---|---|---|
| Wokwi projects from VS Code (HW1–HW7) | `wokwi/` | `diagram.json`, `main.cpp`, `platformio.ini`, `wokwi.toml` |
| HW4 report: I2C analysis in PulseView | `pulseview/` | `HW4_I2C_Test_Report_EN.pdf` |
| Autotest code (HW6) | `pytest/` | https://github.com/alexandramoroz12/embedded-qa-portfolio/tree/main/pytest |
| HW7 report and logs (network, memory stability) | `network-stress/` | `HW7_IoT_Network_Test_Report_EN.pdf`, `platformio-device-monitor-*.log` |
| Full test reports HW1, HW2, HW3, HW5, HW6 | `test-reports/` | `HW*_..._Test_Report_EN.pdf` |
| Bug reports per topic, with screenshots, `WDT_hang.mp4` | `bug-reports/` | `Bug_Reports_HW1…HW7_*_EN.pdf` |
