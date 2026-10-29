"""
Діагностика зв'язку з Wokwi через RFC2217 (без pytest і без Device).

Запуск:  python check.py
"""
import time
import serial

PORT = "rfc2217://localhost:4000"
BAUDRATE = 115200
TIMEOUT = 3.0      # скільки секунд чекати відповідь на одну команду
ATTEMPTS = 3       # скільки разів повторити кожен варіант

# (назва, байти, які відправляємо)
VARIANTS = [
    ("PING  \\n",   b"PING\n"),
    ("PING  \\r\\n", b"PING\r\n"),
    ("ping  \\n",   b"ping\n"),
    ("STATUS \\n",  b"STATUS\n"),   # для порівняння: ця команда працює
]


def drain(ser):
    """Прочитати й викинути все, що вже прийшло (без reset_input_buffer)."""
    time.sleep(0.2)
    if ser.in_waiting:
        ser.read(ser.in_waiting)


def main():
    print(f"Підключення до {PORT} ...")
    ser = serial.serial_for_url(PORT, baudrate=BAUDRATE, timeout=TIMEOUT)
    time.sleep(1.0)
    drain(ser)

    stats = {name: 0 for name, _ in VARIANTS}

    for attempt in range(1, ATTEMPTS + 1):
        print(f"\n--- спроба {attempt} ---")
        for name, data in VARIANTS:
            drain(ser)
            start = time.time()
            ser.write(data)
            ser.flush()
            reply = ser.readline()
            elapsed = time.time() - start

            if reply:
                stats[name] += 1
                print(f"{name:12} -> {reply!r}  ({elapsed:.2f} с)")
            else:
                print(f"{name:12} -> ТИША (чекали {elapsed:.2f} с)")

    ser.close()

    print("\n=== Підсумок (скільки відповідей з %d) ===" % ATTEMPTS)
    for name, _ in VARIANTS:
        print(f"{name:12} {stats[name]}/{ATTEMPTS}")


if __name__ == "__main__":
    main()