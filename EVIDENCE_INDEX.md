# Evidence Package Index

Author: Alexandra Holoborodko · Date: 31.10.2026
Paths are relative to the repository root (structure: see `README.md`). Risks R01–R12 are from `FINAL_TEST_MATRIX.md`. Bug reports are in `bug-reports/` (one file per topic, with screenshots), test reports in `test-reports/`.

| Evidence ID | Related Test/Risk | Type | Description | File/Link |
|---|---|---|---|---|
| E01 | R01 | Serial log + Logic Analyzer | Debounce: RAW → LOGICAL → ACTION on GPIO18. TC-01…04 passed; TC-05/06 failed: NOISE creates a false logical event and action (HW2 Bug_1, Bug_2) | `test-reports/HW2_Debounce_Test_Report_EN.docx`, `bug-reports/Bug_Reports_HW2_Debounce_EN.docx` |
| E02 | R02 | Serial log + Pytest | CLI: 19 TC (HW3), 7 defects: SYSTEM FAULT on input ≥41 characters and on `set` in DIRTY, `speed`/`timeout` bounds. Pytest (HW6): 24 tests, 20 passed, 4 failed; `SET PWM 100` rejected | `test-reports/HW3_UART_CLI_Test_Report_EN.docx`, `test-reports/HW6_Pytest_Test_Report_EN.docx`, `bug-reports/Bug_Reports_HW3_UART_CLI_EN.docx`, `pytest/`, (https://github.com/alexandramoroz12/embedded-qa-portfolio/tree/main/pytest) |
| E03 | R03 | Logic trace (PulseView) | I2C, MPU6050 0x68, SDA 21 / SCL 22: ACK/NACK; after the NACK, `Error -1` and the main loop stops (HW4 Bug). SPI not covered | `pulseview/HW4_I2C_Test_Report_EN.docx`, `bug-reports/Bug_Report_HW4_I2C_EN.docx` |
| E04 | R04, R10 | Reset log + video | `hang` → `[FAULT]` at ≈11:52.4, reboot after ≈3 s by the Wokwi timer (within the requirement); `reset_reason=INT_WDT`; repeated recovery ×10 is stable; the relay stays ON after the WDT (HW5 Bug_3). During the hang the simulator slows to ≈2% speed, so time is taken from the Wokwi timer, not the wall clock | `test-reports/HW5_Recovery_Test_Report_EN.docx`, `bug-reports/Bug_Reports_HW5_Recovery_EN.docx`, `wokwi/WDT_hang.mp4` |
| E05 | R05 | Before/after log | `powerloss` during `save`: `counter` changed, `relay` did not. No conclusion; real hardware is needed | `test-reports/HW5_Recovery_Test_Report_EN.docx` (power-loss scenario) |
| E06 | R06 | Network log | `NET DOWN/UP`, Scenario A: reconnect works, but backoff 500→50 ms (≈20 attempts/s); after NET UP every buffered message is published twice (HW7 BUG-01, BUG-02) | `network-stress/HW7_IoT_Network_Test_Report_EN.docx`, `network-stress/platformio-device-monitor-*.log`, `bug-reports/Bug_Reports_HW7_Network_EN.docx` |
| E07 | R07 | Message IDs | Scenarios B/C, buffer 8/9/12: with >8 messages some IDs are lost, `dropped=0` (HW7 BUG-03) | `network-stress/HW7_IoT_Network_Test_Report_EN.docx` (Scenarios B/C) |
| E08 | R08 | Metrics | Scenario D, `STRESS 100` ×10: ≈80 B per message, −80268 B (≈23%) over 1000 messages (HW7 BUG-04) | `network-stress/HW7_IoT_Network_Test_Report_EN.docx` (Memory Stability Log) |
| E09 | R09 | Test sheet + bug log | Smart Gate Controller, 27 TC: 23 passed, 3 failed, 1 blocked; Bug_1 (state does not become OPEN after `LIMIT_OPEN`), Bug_2 (no ERROR after >10 s in OPENING), Bug_3 (no ERROR with both limits active) | `test-reports/HW1_Smart_Gate_Test_Report_EN.docx`, `bug-reports/Bug_Reports_HW1_Smart_Gate_EN.docx`, `bug-reports/Bug_Reports_HW1.xlsx` |
| E10 | R10 | Serial log + Pytest | Relay ON after the WDT (HW5 Bug_3); `RESET` leaves PWM=255 and LED ON (HW6 Bug_2) | `bug-reports/Bug_Reports_HW5_Recovery_EN.docx`, `bug-reports/Bug_Reports_HW6_Pytest_EN.docx`, `test-reports/HW6_Pytest_Test_Report_EN.docx` |
| E11 | R11 | Serial log | Disconnected DHT22: `[SENSOR] temp=0.0C humidity=0.0%` reported as valid data (HW5 Bug_4) | `bug-reports/Bug_Reports_HW5_Recovery_EN.docx`, `test-reports/HW5_Recovery_Test_Report_EN.docx` |
| E12 | R12 | Serial log + Pytest | `sleep 5` wakes in <1 s (HW5 Bug_1); `GET TEMP` returns JSON without `}` (HW6 Bug_3); `PING` answers in ≈1.3 s against a 1.0 s client timeout (HW6 Bug_4, classified as communication) | `bug-reports/Bug_Reports_HW5_Recovery_EN.docx`, `bug-reports/Bug_Reports_HW6_Pytest_EN.docx` |

## Rule
Evidence must allow another person to verify the QA conclusion.

To verify any conclusion: find the risk in `FINAL_TEST_MATRIX.md` → open its Evidence ID here → open the file and compare the observed fact with the Expected column.

## Notes
- Wokwi projects (`diagram.json`, `main.cpp`, `platformio.ini`, `wokwi.toml`) for HW1–HW7 are in `wokwi/`.
- Time in logs and video is measured by the Wokwi timer or by log timestamps. Wall-clock time is not used, because Wokwi can slow down to ≈2% speed.
- The withdrawn defect HW5 Bug_2 (WDT ≈30 s) is not part of the evidence: see the note in `bug-reports/Bug_Reports_HW5_Recovery_EN.docx` and appendix A of `MASTER_TEST_PLAN.md`.
