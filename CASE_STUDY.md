# Embedded QA Portfolio Case Study

Author: Alexandra Holoborodko · 31.10.2026

## 1. Project
A reference IoT controller based on the ESP32: CLI over UART, DHT22 and MPU6050 sensors (I2C), relay, PWM, LED, and telemetry over Wi-Fi/MQTT. During the course I tested its subsystems on Wokwi set-ups (Smart Gate Controller on Arduino Uno, ESP32 DevKit, `homework_buggy`) as a black box: the firmware deliberately contained defects (BUGGY builds) and I never saw the source code. The final task: combine seven practices into one Master Test Plan and give a release decision.

## 2. My QA Goal
To give the team an evidence-based answer to one question: **can this firmware be released?** For that I had to show which risks are really critical, what verifies each of them, what evidence proves the conclusion, and what remains unverified.

## 3. System Overview
```
 INPUTS                       ESP32 FIRMWARE                      OUTPUTS
 CLI (UART) ──────────┐                                     ┌── Relay / PWM / LED
 Sensors (DHT22, I2C) ─┼──►  parser · state machine ·  ──────┼── MQTT telemetry
 Button / limit switch ┤      WDT · offline buffer · NVS     └── Serial logs / STATUS
 Network (Wi-Fi / MQTT)┘
```
Interfaces: GPIO, UART (115200), I2C, Wi-Fi, MQTT. Dependencies: power supply, MQTT broker, network, Wokwi. Critical data and actuators: relay, PWM, LED; configuration (`speed`, `timeout`, `counter`); unique message IDs; free heap.

## 4. Critical Risks
Priority = probability × impact (scale 1–3). The risks I consider critical, and why:

| Risk | Why it is critical |
|---|---|
| Actuators in an unsafe state after a fault/reset | Direct safety impact: relay ON without a command |
| Silent data loss in the offline buffer | The user never learns about the loss (`dropped=0`) |
| Memory leak | The device will eventually fail, but it is meant to run for a long time |
| Sensor/peripheral failure without a controlled state | Wrong data (`temp=0.0` as valid) or a stopped main loop |
| Faulty command handling | One long line reboots the device |

Medium: bounce/noise, timing, JSON format. Full table: section 7 of `MASTER_TEST_PLAN.md`.

## 5. Test Strategy
Risks are turned into levels, methods and environments (full matrix: `FINAL_TEST_MATRIX.md`):

| Level | Method | Environment | Risks |
|---|---|---|---|
| Component / interface | BVA, negative, input length 40/41 | Wokwi + Serial | CLI, response format |
| Protocol | I2C trace analysis (ACK/NACK) | Wokwi Logic Analyzer + PulseView | I2C |
| System (black-box) | States, limit switches, timeout | Wokwi | Gate, bounce, sensor |
| Recovery / fault injection | `hang`, `powerloss`, FAULT ON/OFF | Wokwi | WDT, persistence, sensor |
| Network / reliability | NET DOWN/UP, buffer limit, `STRESS` | Wokwi + PlatformIO | Network, buffer, heap |
| Automation | Pytest over RFC2217 | Python | Regression |

Why these tests: every risk has a test that reproduces it in an observable way (command → response → log/trace/metric). Test levels go from cheap (parser) to expensive (network, soak).

## 6. Key Scenarios
1. **Hang → WDT → relay state (HW5).** The `hang` command, then observing the reset and the outputs. The reset happens in ≈3 s, but the relay stays ON: Critical.
2. **Offline buffer overflow (HW7, Scenario C).** Sending 8/9/12 messages with no network, then NET UP. Some IDs are lost and `dropped=0`: silent data loss.
3. **Memory Stability Log (HW7, Scenario D).** `STRESS 100` ×10 with heap after each iteration: −80268 B (≈23%) over 1000 messages, ≈80 B per message.
4. **Long input and DIRTY state (HW3).** Input of 40/41 characters; `set` in DIRTY → SYSTEM FAULT.
5. **Loss of the I2C device (HW4).** FAULT ON → NACK, `Error -1` in the log, the main loop stops, recovery only after a restart.
6. **Noise on the input (HW2).** The NOISE signal creates a false logical event and action, also during a critical operation.

## 7. Automation
Pytest over RFC2217 to Wokwi: `test_boundaries`, `test_relay`, `test_sensor`, `test_status`, `test_timeout`, `test_my_cases` — 24 tests (20 passed, 4 failed). Code: `pytest/`. I automated checks that are repeatable and deterministic: value bounds, response format, relay and PWM state, status. What needs observation (PulseView, video) stays manual.

An important lesson: the `PING` test failed because of the 1.0 s client timeout although the response was correct (≈1.3 s). It was classified as communication, not as a firmware defect. Before a bug report, every `FAILED` goes through triage: test logic → environment → communication → requirement → firmware.

Next candidates for automation: HW7 network scenarios (`NET DOWN`, `SEND`, `BUFFER`, `STATUS`, `STRESS`), the Δheap check, timing measurements from log timestamps; mark known failures with `xfail(strict=True)` and the defect ID.

## 8. Fault Injection
| Fault | How it was injected | How recovery was verified |
|---|---|---|
| Hang | `hang` | `reset_reason=INT_WDT`, time by the Wokwi timer, output state, repeated ×10 |
| Power loss | `powerloss` during `save` | `counter` / `relay` before and after (no conclusion) |
| Sensor disconnect | Disconnect DHT22 / I2C FAULT ON → OFF | Is there a controlled error state, and does exchange recover |
| Sleep | `sleep 5` | Wake-up time |
| Network loss | `NET DOWN` / `NET UP` | Backoff, reconnect, buffer, duplicates |
| Load | `STRESS 100` ×10 | Heap trend |

## 9. Evidence
Logs, traces, screenshots, test reports, metrics. The index with paths: `EVIDENCE_INDEX.md`. Rule: evidence must allow another person to verify the conclusion. Reports sit in `test-reports/`, `network-stress/` and `pulseview/`, bug reports in `bug-reports/`.

## 10. Findings
24 defects: 12 Critical, 2 High, 8 Major, 2 Low/Medium. The register is appendix A of `MASTER_TEST_PLAN.md`. The most important:

| Finding | Impact |
|---|---|
| Relay ON after the WDT; `RESET` leaves PWM=255 and LED ON | Actuators in an unsafe state |
| `dropped=0` while IDs are lost; duplicates after NET UP | Silent data loss and duplication |
| Leak of ≈80 B/message | Memory exhaustion after ≈4400 messages (extrapolation from simulation) |
| SYSTEM FAULT on long input and `set` in DIRTY | A reboot from a single command |
| I2C does not recover; `temp=0.0` as valid data | Stopped main loop or wrong readings |

**A withdrawn defect.** In HW5 I first recorded "WDT reset in ≈30 s". Mentor feedback showed that this was a measurement error: 30 s was wall-clock time, because after `hang` Wokwi slows down to ≈2% speed. By the simulation timer `[FAULT]` was at ≈11:52.4 and the reboot at ≈11:55.4, i.e. ≈3 s, which is within the requirement. I removed that defect and added a rule: time is measured by the Wokwi timer or by log timestamps. The same rule applies to `sleep` and `PING`, which I still need to re-check.

## 11. Regression
- **Smoke:** boot → `help`, `status` → sensor → network CONNECTED.
- **Critical regression:** `SET PWM 0/100`, `RESET` → safe state; `speed`/`timeout` bounds; input of 40/41 characters; `set` in DIRTY; relay after `hang`; disconnected DHT22; I2C FAULT ON → OFF; Scenarios B/C (8/9/12) checking `sent`, `dropped`, `duplicates`; Scenario D; ERROR on timeout and on two limits.
- **Full regression:** all scenarios of HW1–HW7.

## 12. Limitations
Wokwi simulates logic, not physics. The simulation does not prove:
- real contact bounce, electrical noise, deep-sleep consumption;
- storage integrity on a real power cut (power loss during `save`);
- real Wi-Fi timing (a failed reconnect takes ≈45 ms in the simulator and seconds in reality) and broker behaviour on duplicates;
- long soak tests: the ≈4400-message threshold is only an extrapolation.

Not covered: SPI, security, OTA, BLE. The results apply to the specific BUGGY builds.

## 13. Release / Quality Conclusion
### **NO-GO**
Critical safety and data-integrity requirements are violated (R02, R03, R07, R08, R09, R10, R11 in the matrix) and Exit Criteria 2–5 are not met. Conditions for moving to GO WITH RISKS: all Critical defects fixed and retested, the critical regression passes at 100%, Major defects fixed or accepted by the product owner, and a plan to verify the residual risks on real hardware. Full decision: `RELEASE_DECISION.md`.

## 14. What I Would Improve Next
1. Retest after fixes with new evidence for every Critical defect.
2. Automate the HW7 network scenarios and the Δheap check; mark known failures `xfail(strict=True)`.
3. Verify power loss during `save`, deep sleep and bounce on real hardware.
4. Run an hours-long soak test with heap and watchdog monitoring.
5. Re-check `sleep` and `PING` timing against the Wokwi timer rather than the wall clock.
6. Close coverage gaps: SPI, security, OTA.
