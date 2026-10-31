# Final Test Matrix

Product: reference IoT controller on ESP32 (see `MASTER_TEST_PLAN.md`).
Statuses are based on the results of HW1–HW7 (Wokwi, BUGGY builds), as of 31.10.2026.

**Status legend:** `Failed` — open defects · `Passed` — the requirement is met · `Not verified` — no conclusion, real hardware needed.

| ID | Quality Risk | Level | Test / Method | Environment | Expected | Evidence | Regression? | Status |
|---|---|---|---|---|---|---|---|---|
| R01 | Button bounce creates duplicate action | System | Repeated input, NOISE on GPIO18: RAW → LOGICAL → ACTION (HW2) | Wokwi (ESP32 DevKit v1); real HW not tested | 1 event per press | counter/log, Logic Analyzer; E01. TC-01…04 passed | Yes | **Failed** — TC-05/06: NOISE creates a false logical event and action, also during a critical operation (HW2 Bug_1, Bug_2; 2 Critical) |
| R02 | UART command parsing fails | Interface | CLI positive/negative, BVA, input length 40/41, state sequences (HW3); pytest (HW6) | Wokwi + Python (pytest, `rfc2217://localhost:4000`) | structured response | Serial log + pytest output; E02 | Yes | **Failed** — SYSTEM FAULT on input ≥41 characters and on `set` in DIRTY; `set speed 256` / `set timeout 0` accepted; invalid commands return `OK`; `SET PWM 100` rejected (HW3: 19 TC, 7 defects; HW6 Bug_1) |
| R03 | I2C/SPI transaction corrupted | Protocol | Protocol trace analysis: ACK/NACK, peripheral loss (MPU6050 0x68), FAULT ON/OFF (HW4) | Wokwi Logic Analyzer → `.vcd`, PulseView | valid frames | trace (`.vcd`, PulseView screenshots) + `Wire.cpp` log; E03 | Yes | **Failed (I2C)** — after a NACK, `Error -1`: the main loop stops and recovers only after a restart (HW4 Bug, Critical). **SPI not covered** |
| R04 | Firmware hangs | Recovery | WDT fault injection: `hang`, repeated recovery ×10 (HW5) | Wokwi (ESP32 DevKit V1) | controlled recovery | reset log (`reset_reason`), Wokwi timestamps, video; E04 | Yes | **Passed** — `[FAULT]` → reboot ≈3 s by the Wokwi timer, `reset_reason=INT_WDT`, 10/10 recoveries. (Actuator state after the WDT: see R10) |
| R05 | Critical state corrupted | Reliability | Interrupted save: `powerloss` during `save` (HW5) | Wokwi; real HW needed | valid old/new state | before/after log (`counter`, `relay`); E05 | Yes | **Not verified** — `counter` changed, `relay` did not; no conclusion, not reproducible in simulation |
| R06 | Network unavailable | Network | NET DOWN/UP, Scenario A (HW7) | Simulation (Wokwi + PlatformIO) | controlled reconnect | network log (`[NET]`, `[MQTT]`), `STATUS` JSON; E06 | Yes | **Failed** — reconnect works, but backoff 500→50 ms (≈20 attempts/s, reconnect storm); after NET UP every buffered message is published twice (HW7 BUG-01, BUG-02; Major) |
| R07 | Buffer overflow loses data silently | Network | Offline overflow: Scenarios B/C, buffer 8/9/12 (HW7) | Simulation | defined policy | IDs/counters (`sent`, `dropped`, `duplicates`); E07 | Yes | **Failed** — with >8 messages some IDs are lost, `dropped=0`, the flush is corrupted (HW7 BUG-03; Critical) |
| R08 | Long-run memory degradation | Reliability | Repeated stress: `STRESS 100` ×10, heap trend, Scenario D (HW7) | Simulation | stable trend | memory log (heap, Δheap); E08 | Yes | **Failed** — ≈80 B per message, −80268 B (≈23%) over 1000 messages (HW7 BUG-04; Critical) |
| R09 *(added)* | Gate controller enters an unsafe/undefined state | System | Black-box state machine, limit switches, timeout, simultaneous limits (HW1, 27 TC) | Wokwi (Arduino Uno, Smart Gate Controller, Build_1) | defined state transitions; ERROR; relays OFF | Hardware Test Sheet / Test Log, Bug Log; E09 | Yes | **Failed** — 23 passed, 3 failed, 1 blocked: state does not become OPEN after `LIMIT_OPEN` (Critical); no ERROR after >10 s in OPENING or with both limits active (High) |
| R10 *(added)* | Actuators left in an unsafe state after reset/fault | System | `RESET`, WDT, ERROR logic (HW5, HW6) | Wokwi + Python | relay OFF, PWM 0, LED OFF | Serial log + pytest output; E10 | Yes | **Failed** — relay ON after the WDT (HW5 Bug_3); `RESET` leaves PWM=255 and LED ON (HW6 Bug_2); both Critical |
| R11 *(added)* | Sensor failure reported as valid data | System | Disconnect DHT22, `sensor` command (HW5) | Wokwi (ESP32 DevKit V1) | explicit sensor error, no valid measurement | Serial log; E11 | Yes | **Failed** — `[SENSOR] temp=0.0C humidity=0.0%` returned as data (HW5 Bug_4; Critical) |
| R12 *(added)* | Timing and response-format deviations | System / Automation | `sleep 5` timing (HW5); `GET TEMP` JSON validation and `PING` latency (HW6) | Wokwi + Python | wake-up in 4.25–5.75 s; valid JSON; responses within the client timeout | Serial log + pytest output; E12 | Yes | **Failed** — `sleep 5` wakes in <1 s (Major); `GET TEMP` JSON has no `}` and `PING` takes ≈1.3 s (Low/Medium; the latter classified as communication) |

## Summary

| Status | Count | Risks |
|---|---|---|
| Failed | 10 | R01, R02, R03, R06, R07, R08, R09, R10, R11, R12 |
| Passed | 1 | R04 |
| Not verified | 1 | R05 |

All 12 risks are part of the regression (see section 13 of `MASTER_TEST_PLAN.md`). Open defects (12 Critical, 2 High) require fixes and a retest with new evidence. Recommendation: **NO-GO**.

## Notes
- R09–R12 were added to the template's R01–R08 so that the matrix also covers HW1 (gate state machine), the safe state of actuators, sensor failure handling and timing/format findings. If exactly 8 rows are required, R09 can be merged into R01/R04 (System level), R10 into R04, R11 into R03 and R12 into R02.
- In the template's Environment column, `Wokwi/real` was filled in factually: real hardware was not used in the course.
- Defect IDs match appendix A of `MASTER_TEST_PLAN.md`.
