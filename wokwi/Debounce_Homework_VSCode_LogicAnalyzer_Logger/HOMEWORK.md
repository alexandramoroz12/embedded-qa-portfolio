# Homework — Button Bouncing / Debounce

## Мета

Провести black-box Embedded QA тестування interrupt-driven кнопки та перевірити, чи firmware правильно відокремлює **RAW GPIO transitions** від **logical events** та **actions**.

## Acceptance criteria

1. Одне фізичне натискання → рівно один logical press event.
2. Bouncing може створювати багато RAW transitions / interrupts, але не додаткові logical events.
3. Утримання кнопки не повинно генерувати повторні press events.
4. Короткий нестабільний імпульс, що не проходить debounce requirement, не повинен запускати action.
5. Під час critical operation шум не повинен створювати додаткову logical action.

## Обов'язкові сценарії

1. Normal press.
2. Button bounce: порівняти `D0 RAW` і `D1 LOGICAL` у Logic Analyzer.
3. Long press 2–5 s.
4. 5 rapid real presses.
5. Short noise pulse.
6. Noise during critical operation.
7. Повторити проблемний сценарій мінімум 5 разів та визначити reproducibility.

## Logger — обов'язкова частина evidence

У firmware вже є вбудований **Serial Logger**. Не змінюйте його до завершення black-box тестування.

Logger допомагає порівняти чотири рівні:

**Physical action → RAW interrupts → LOGICAL events → ACTIONS**

Після прийнятого натискання logger виводить:

```text
LOGICAL EVENT #N
Button press ACCEPTED
RAW interrupts total: N
RAW interrupts since previous logical event: N
LOGICAL events total: N
ACTIONS total: N
ACTION executed ONCE
LOGGER: physical press -> RAW -> LOGICAL -> ACTION
```

Періодична статистика має формат:

```text
[LOGGER] RAW interrupts: N | Logical events: N | Actions: N
```

### Що зафіксувати

Для кожного основного сценарію запишіть:

| Scenario | Physical actions | RAW interrupts / D0 | Logical events / D1 | Actions | Result |
|---|---:|---:|---:|---:|---|
| Normal press | 1 | ? | ? | ? | PASS/FAIL |
| Bounce | 1 | ? | ? | ? | PASS/FAIL |
| Long press | 1 | ? | ? | ? | PASS/FAIL |
| 5 rapid presses | 5 | ? | ? | ? | PASS/FAIL |
| Noise pulse | 0 | ? | ? | ? | PASS/FAIL |
| Noise during critical operation | 0/1 | ? | ? | ? | PASS/FAIL |

**Важливо:** RAW interrupts можуть бути більшими за кількість фізичних натискань — це саме по собі не є дефектом. Для QA важливо, чи перетворився шум на зайву logical event/action.

## Evidence

Для кожного FAIL додайте мінімум один доказ:

- screenshot Logic Analyzer;
- Serial Logger output;
- або обидва для ключових сценаріїв.

Для кожного важливого сценарію бажано показати зв'язок між D0, D1 та Logger.

## Що здати

1. Test Report / Hardware Test Sheet.
2. Logger evidence table.
3. Bug Report для кожного підтвердженого унікального дефекту.
4. Logic Analyzer screenshot для сценарію bouncing.
5. QA Conclusion: `PASS / FAIL / needs clarification` + 2–3 речення чому.

## Bonus — Boundary Testing

Дослідити стабільні/шумові імпульси біля debounce boundary: **1, 5, 10, 20, 29, 30, 31, 50 ms**.

Зафіксуйте:

- тривалість/тип імпульсу;
- RAW interrupts;
- logical events;
- action;
- PASS/FAIL.

Якщо точну тривалість неможливо надійно створити вручну у Wokwi — позначте це як **testability limitation**, а не як FAIL firmware.
