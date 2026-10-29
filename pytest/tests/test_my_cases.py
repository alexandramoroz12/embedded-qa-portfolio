import pytest
import time

# Вимога 6: PWM поза діапазоном 0..100 -> INVALID_VALUE
@pytest.mark.parametrize("value", ["abc", "@#$%", "1.5"])
def test_pwm_invalid_values(device, value):
    r = device.command(f"SET PWM {value}")
    assert r["status"] == "ERROR"
    assert r["code"] == "INVALID_VALUE"


# Вимога 8: невідомі команди -> UNKNOWN_COMMAND
@pytest.mark.parametrize("cmd", ["FOO", "ping", "SET LED", "SET PWM", "GET TEMPX"])
def test_unknown_commands_return_structured_error(device, cmd):
    r = device.command(cmd)
    assert r["status"] == "ERROR"
    assert r["code"] == "UNKNOWN_COMMAND"

# ---------- Пункт 5: перевірка timeout ----------
def test_timeout_when_no_response(device):
    start = time.time()                       # 1. запам'ятали час старту
    with pytest.raises(TimeoutError):         # 2. чекаємо помилку TimeoutError
        device.command("NO_RESPONSE")         # 3. шлемо команду, на яку пристрій мовчить
    elapsed = time.time() - start             # 4. рахуємо, скільки пройшло часу
    assert elapsed < 3.0                      # 5. перевіряємо: менше 3 секунд


def test_device_works_after_timeout(device):
    with pytest.raises(TimeoutError):                    # 1. знову викликаємо мовчання
        device.command("NO_RESPONSE")
    assert device.command("STATUS")["status"] == "OK"    # 2. пристрій досі живий?

# --------------------
