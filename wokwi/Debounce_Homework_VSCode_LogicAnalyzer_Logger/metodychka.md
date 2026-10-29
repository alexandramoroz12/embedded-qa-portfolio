# Методичка — GPIO Interrupt, Button Bouncing та Software Debounce

## 1. Що тестуємо

Ланцюжок спостереження:

```text
BUTTON → RAW GPIO18 → INTERRUPT/ISR → SOFTWARE DEBOUNCE → LOGICAL EVENT → GPIO2/LED
```

QA перевіряє не кількість interrupt як таку, а чи перетворюється одна фізична дія на одну логічну дію.

## 2. Logic Analyzer

У схемі є Logic Analyzer:

- **D0 = RAW GPIO18** — фізичний сигнал кнопки;
- **D1 = GPIO2** — видима реакція на прийняту logical event.

При bouncing на D0 можна побачити кілька швидких переходів. Це ще не bug. Питання QA: чи породили вони зайву logical action на D1/у логах.

## 3. Serial evidence

Firmware виводить `LOGICAL EVENT`, `RAW interrupts total` та `ACTION executed ONCE`. Використовуйте ці дані разом із Logic Analyzer як evidence.

## 4. Black-box правило

До завершення тестування не змінюйте `src/main.cpp` і не використовуйте значення констант із коду як заміну фактичному тесту. Спочатку спостерігайте поведінку системи.

## 5. Що є дефектом

- 1 physical press → 2+ logical press events/actions;
- короткий noise → false logical event;
- long press → повторні press events без вимоги;
- окремі валідні швидкі натискання губляться;
- noise під час critical operation запускає зайву action.

Багато RAW transitions або ISR executions **самі по собі не є дефектом**.

## 6. Boundary / testability

Для bonus перевіряйте значення біля debounce boundary. Якщо Wokwi GUI не дозволяє надійно сформувати імпульс заданої тривалості, це треба записати як **testability/environment limitation**. Не ставте firmware FAIL без відтворюваного input.
