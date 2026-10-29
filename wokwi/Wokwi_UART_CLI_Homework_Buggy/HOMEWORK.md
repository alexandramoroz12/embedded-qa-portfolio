# Домашня робота — UART / CLI Robustness Testing

## Мета
Самостійно провести QA-тестування CLI мікроконтролера та знайти дефекти обробки вводу.

## Умови
Вам надано buggy build. Не змінюйте firmware до завершення тестування.

## Потрібно

1. Ознайомитися з `REQUIREMENTS.md`.
2. Скласти щонайменше **15 власних test inputs**.
3. Обов'язково використати:
   - Boundary Value Analysis;
   - Negative Testing;
   - Input Length Testing;
   - State / Sequence Testing;
   - Stress Input.
4. Знайти мінімум **3 різні дефекти**.
5. Для кожного дефекту визначити:
   - точні Steps to Reproduce;
   - Expected Result;
   - Actual Result;
   - Severity;
   - Reproducibility;
   - Evidence.
6. Окремо визначити точну межу довжини рядка, після якої поведінка змінюється.
7. Після кожного негативного тесту перевіряти `status`.

## Заборонено
- починати з аналізу вихідного коду;
- змінювати firmware до завершення тестування;
- описувати дефект без відтворення.

## Здати
- `Homework_Test_Report.docx`;
- мінімум 3 Bug Reports;
- Serial log / screenshots;
- короткий QA Conclusion.

## Bonus
Знайти дефект, який проявляється лише після певної попередньої валідної команди.
