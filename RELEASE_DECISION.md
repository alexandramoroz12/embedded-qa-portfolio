# Release Decision

**Product:** reference IoT controller on ESP32 (BUGGY builds, HW1–HW7)
**Author:** Alexandra Holoborodko · **Date:** 31.10.2026

## Decision: NO-GO

Release is not recommended. Safety and data-integrity requirements are violated, critical defects are open, and no retest has been performed.

## Rationale (evidence → conclusion)

| Blocking risk | What proves the conclusion | Evidence |
|---|---|---|
| R10 Actuators unsafe after fault/reset | Relay ON after the WDT reset; `RESET` leaves PWM=255 and LED ON | E04, E10 |
| R09 Gate state machine | No ERROR after >10 s in OPENING or with both limits active; state does not become OPEN after `LIMIT_OPEN` | E09 |
| R07 Silent data loss | With >8 messages some IDs are lost, `dropped=0` | E07 |
| R08 Memory leak | ≈80 B/message, −23% heap over 1000 messages | E08 |
| R03 Peripheral failure | After a NACK on I2C the main loop stops; recovery only after a restart | E03 |
| R11 Invalid sensor data | Disconnected DHT22 reported as valid `temp=0.0 humidity=0.0` | E11 |
| R02 Input robustness | SYSTEM FAULT on input ≥41 characters and on `set` in DIRTY | E02 |
| R01 False events | NOISE creates a logical event and action, also during a critical operation | E01 |

## Exit Criteria status

| # | Criterion | Status |
|---|---|---|
| 1 | 100% of tests for risks with Priority ≥7 executed | Executed in simulation; not on real hardware |
| 2 | 0 open Blocker/Critical | **Not met:** 12 Critical + 2 High open |
| 3 | Regression 100% | **Not met:** 4 of 24 pytest tests failed |
| 4 | Recovery confirmed | **Partial:** WDT reset time is fine (≈3 s) and the network recovers, but I2C and the relay state after the WDT do not meet the requirements |
| 5 | Heap: deviation ≤1% after 1000 messages *(proposed threshold)* | **Not met:** −23% |
| 6 | Evidence stored | **Met** |
| 7 | Risks accepted or documented | Documented; no formal acceptance |

## What works
- Repeated recovery after `hang` is stable (10 of 10).
- The network recovers after `NET DOWN/UP` (the problems are the attempt rate and duplicates, not a missing reconnect).
- Debounce on regular scenarios (TC-01…04) and gate transitions to CLOSED/OPENING/STOPPED work.

## Conditions for moving to GO WITH RISKS
1. All Critical defects are fixed and pass a retest with new evidence.
2. The critical regression passes at 100%.
3. Major defects are fixed or accepted as known risks by the product owner.
4. There is a plan to verify the residual risks on real hardware.

## Residual risks (not verified)
Power loss during `save` (no conclusion), deep-sleep current and wake-up, real bounce and noise, real Wi-Fi/MQTT, soak tests, automated tests on real hardware; not covered: SPI, security, OTA, BLE.

## Who decides
The product owner and the test manager, based on `EVIDENCE_INDEX.md` and this document. The decision is reviewed after the retest.
