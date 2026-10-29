# REQUIREMENTS — UART CLI Homework Device

## Commands
REQ-01. CLI supports:
- `help`
- `status`
- `set speed <value>`
- `set timeout <value>`
- `led on`
- `led off`
- `save`
- `reset`

## Speed
REQ-02. `speed` accepts integer values from **0 to 255 inclusive**.
REQ-03. Values outside 0..255 must return an ERROR and must not modify speed.

## Timeout
REQ-04. `timeout` accepts integer values from **1 to 60 inclusive**.
REQ-05. Values outside 1..60 must return an ERROR and must not modify timeout.

## Parsing
REQ-06. Commands are case-sensitive.
REQ-07. Unknown or incomplete commands must return a controlled ERROR.
REQ-08. Special-character-only input must not be treated as a valid command.
REQ-09. Invalid commands must not change device configuration.

## Input length
REQ-10. Maximum CLI input length is **40 characters**.
REQ-11. Any line longer than 40 characters must return `ERROR: command too long`.
REQ-12. Long input must not cause reboot, hang or parser corruption.

## State / robustness
REQ-13. Invalid input must behave the same regardless of previous valid commands.
REQ-14. A DIRTY configuration state must not make invalid commands dangerous.
REQ-15. After every validation error, `status` must remain available.
REQ-16. User input must never cause an unexpected reboot or crash.
