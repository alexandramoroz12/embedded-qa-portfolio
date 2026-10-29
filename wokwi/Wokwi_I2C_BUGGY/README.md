# Wokwi — I2C Fault Injection (BUGGY)

## Завдання студента
Знайти дефект **як black-box / protocol-analysis проблему**, а не просто прочитати код.

## Склад стенду
- ESP32 DevKit
- MPU6050, address `0x68`
- Logic Analyzer: D0=SDA, D1=SCL
- FAULT slide switch

## Сценарій
1. Запусти simulation.
2. Переконайся, що `[DATA] Temperature` та `[SYSTEM] Main loop alive` з'являються.
3. Дай системі попрацювати 3–5 секунд.
4. Перемкни FAULT у ON.
5. Спостерігай Serial Monitor та I2C traffic.
6. Через кілька секунд поверни FAULT у OFF.
7. Перевір, чи відновилась система.
8. Зупини simulation та проаналізуй `i2c-capture.vcd` у PulseView.

## Питання
- Який останній успішний ACK?
- Де з'являється перший NACK?
- Чи є retry limit?
- Чи є error state?
- Чи продовжує працювати main loop?
- Чи відновлюється система без reboot?

## Прихований очікуваний дефект
Під час I2C failure firmware входить у нескінченне повторення / очікування.
Через blocking logic інші функції main loop перестають виконуватись.

Не показуй цей README студентам до завершення пошуку дефекту, якщо хочеш провести вправу як справжній black-box challenge.
