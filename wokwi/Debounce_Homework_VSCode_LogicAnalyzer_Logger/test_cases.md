# Suggested Test Cases — Button Debounce

> Це стартовий набір. Слухач може додавати власні negative/boundary scenarios.

| ID | Scenario | Expected |
|---|---|---|
| TC-01 | Press/release `BUTTON` once | 1 physical press → 1 logical event → 1 action |
| TC-02 | Observe one normal press in Logic Analyzer | D0 may contain multiple transitions; D1 changes only once for accepted press; Logger records one logical event/action |
| TC-03 | Hold `BUTTON` for 2–5 s | No repeated press events/actions while held |
| TC-04 | Press `BUTTON` 5 separate times | 5 physical presses → 5 logical press events → 5 actions |
| TC-05 | Generate a short `NOISE` pulse | RAW activity is allowed; no false logical press/action |
| TC-06 | Press `BUTTON`, then generate `NOISE` during critical operation | No extra logical event/action; operation continues |
| TC-07 | Repeat the most suspicious scenario 5 times | Record reproducibility and evidence |

## What to record

For every execution record:

- Physical actions
- RAW interrupts / D0 transitions
- Logical events / D1 behavior
- Actions
- Expected Result
- Actual Result
- PASS / FAIL / BLOCKED
- Evidence / Bug ID

## Logger evidence

Use the built-in Serial Logger to record:

- `RAW interrupts total`
- `RAW interrupts since previous logical event`
- `LOGICAL events total`
- `ACTIONS total`

Compare these values with the number of physical actions and the Logic Analyzer capture.

## Main QA rule

**Physical actions = logical events = actions** for valid button presses.

`RAW transitions > physical actions` can be normal for a mechanical button. The firmware is responsible for filtering instability before it becomes a logical action.
