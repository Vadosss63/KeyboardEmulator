# controller_emulator.py
import argparse
import sys
import threading
import time
from dataclasses import dataclass, field
from typing import List, Tuple, Optional, Dict

try:
    import serial  # pyserial
except ImportError:
    serial = None


# --- Протокол из заголовка ---
PROTOCOL_SOF = 0xAA

# Команды App -> Ctrl
CMD_BTN_PRESSED           = 0x01
CMD_BTN_RELEASED          = 0x02
CMD_MODE_CHECK_KEYBOARD   = 0x03
CMD_MODE_RUN              = 0x04

# Длины «payload+checksum» (т.е. байты после поля Length и ВКЛЮЧАЯ checksum)
# App->Ctrl: Command(1) + Pin1(1) + Pin2(1) + Checksum(1) = 4
A2C_LENGTH = 4
# Ctrl->App: Pin1(1) + Pin2(1) + LEDs(15) + Checksum(1) = 18
C2A_LENGTH = 18

def calc_checksum(data: bytes) -> int:
    return sum(data) & 0xFF


@dataclass
class A2C_Packet:
    command: int
    pin1: int
    pin2: int


@dataclass
class ControllerState:
    mode_check: bool = False     # False = RUN, True = CHECK_KEYBOARD
    # Множество «нажатых» пар (pin1,pin2). Храним нормализованно (меньший, больший), 1..15
    pressed_pairs: set = field(default_factory=set)
    # Последнее «интересное» событие для заполнения pin1/pin2 в CHECK-режиме
    last_event_pins: Tuple[int, int] = (0, 0)

    def set_mode_check(self, flag: bool):
        self.mode_check = flag
        if not flag:
            self.last_event_pins = (0, 0)

    def press(self, p1: int, p2: int):
        key = self._norm_pair(p1, p2)
        self.pressed_pairs.add(key)
        self.last_event_pins = (p1, p2)

    def release(self, p1: int, p2: int):
        key = self._norm_pair(p1, p2)
        self.pressed_pairs.discard(key)
        self.last_event_pins = (p1, p2)

    def compute_leds(self) -> List[int]:
        """
        Простая эвристика: LED[i] = 1, если номер i участвует
        хотя бы в одной нажатой паре; иначе 0.
        """
        active = set()
        for a, b in self.pressed_pairs:
            if 1 <= a <= 15: active.add(a)
            if 1 <= b <= 15: active.add(b)
        leds = [0]*15
        for i in range(15):
            leds[i] = 1 if (i+1) in active else 0
        return leds

    @staticmethod
    def _norm_pair(a: int, b: int) -> Tuple[int, int]:
        return (min(a, b), max(a, b))


class Framer:
    """
    Побайтный парсер пакетов App->Ctrl:
    SOF(0xAA) | Length | Command | Pin1 | Pin2 | Checksum
    """
    def __init__(self, on_packet, verbose=False):
        self.on_packet = on_packet
        self.verbose = verbose
        self.buf = bytearray()
        self.expected_total = None  # полная длина кадра, когда известно

    def feed(self, chunk: bytes):
        self.buf.extend(chunk)
        self._drain()

    def _drain(self):
        while True:
            # ищем SOF
            sof_index = self.buf.find(bytes([PROTOCOL_SOF]))
            if sof_index == -1:
                self.buf.clear()
                return
            if sof_index > 0:
                del self.buf[:sof_index]

            # нужно минимум 2 байта (SOF + Length), чтобы понять ожидаемую длину
            if len(self.buf) < 2:
                return

            # buf[0] == SOF
            length = self.buf[1]
            expected = 1 + 1 + length  # SOF + Length + (payload+checksum)
            if len(self.buf) < expected:
                # ждём остальное
                return

            frame = bytes(self.buf[:expected])
            del self.buf[:expected]

            # Проверка длины для App->Ctrl
            if length != A2C_LENGTH:
                if self.verbose:
                    print(f"[WARN] Bad A2C length={length}, expected {A2C_LENGTH}", file=sys.stderr)
                continue

            # валидация чексумы
            # checksum считается по всем байтам ДО checksum'а (т.е. SOF..Pin2 включительно)
            checksum_expected = frame[-1]
            checksum_calc = calc_checksum(frame[:-1])
            if checksum_expected != checksum_calc:
                if self.verbose:
                    print(f"[WARN] Bad checksum: got=0x{checksum_expected:02X}, calc=0x{checksum_calc:02X}", file=sys.stderr)
                continue

            # распаковка
            # frame: [0]=SOF, [1]=Length, [2]=Command, [3]=Pin1, [4]=Pin2, [5]=Checksum
            cmd = frame[2]
            pin1 = frame[3]
            pin2 = frame[4]
            pkt = A2C_Packet(cmd, pin1, pin2)
            self.on_packet(pkt)


class ControllerEmulator:
    def __init__(self, port: str, baud: int, period_s: float = 0.05, verbose: bool = False):
        if serial is None:
            raise RuntimeError("pyserial не установлен. Установите: pip install pyserial")

        self.port_name = port
        self.baud = baud
        self.period_s = period_s
        self.verbose = verbose

        self.ser: Optional[serial.Serial] = None
        self.framer = Framer(self._on_a2c_packet, verbose=verbose)
        self.state = ControllerState()
        self._stop = threading.Event()
        self._tx_lock = threading.Lock()

        # Для CHECK режима — «затухание» последних пинов (чтобы UI видел вспышку)
        self._check_hold_ms = 150
        self._last_event_time = 0.0

    # --- Lifecycle ---
    def open(self):
        self.ser = serial.Serial(
            self.port_name,
            self.baud,
            timeout=0,           # неблокирующее чтение
            write_timeout=1
        )
        if self.verbose:
            print(f"[INFO] Opened {self.port_name} @ {self.baud}", file=sys.stderr)

    def close(self):
        if self.ser:
            try:
                self.ser.close()
            finally:
                self.ser = None

    def run(self):
        self._stop.clear()
        tx_thread = threading.Thread(target=self._tx_loop, daemon=True)
        tx_thread.start()

        try:
            while not self._stop.is_set():
                self._rx_pump()
                time.sleep(0.001)
        except KeyboardInterrupt:
            pass
        finally:
            self._stop.set()
            self.close()

    # --- RX/TX ---
    def _rx_pump(self):
        if not self.ser:
            return
        data = self.ser.read(1024)
        if data:
            if self.verbose:
                print(f"[RX] {data.hex(' ')}", file=sys.stderr)
            self.framer.feed(data)

    def _tx_loop(self):
        next_time = time.perf_counter()
        while not self._stop.is_set():
            now = time.perf_counter()
            if now >= next_time:
                self._send_status()
                next_time = now + self.period_s
            time.sleep(0.001)

    def _send_status(self):
        """
        Формируем пакет Ctrl->App:
        SOF | Length(=18) | Pin1 | Pin2 | LED[15] | Checksum
        Pin1/Pin2:
            - в CHECK режиме — последние «интересные» пины, но «затухают» через _check_hold_ms
            - в RUN режиме — 0/0
        """
        if not self.ser:
            return

        pin1, pin2 = (0, 0)
        if self.state.mode_check:
            # «вспышка» последних пинов на небольшое время
            if (time.perf_counter() - self._last_event_time) * 1000 <= self._check_hold_ms:
                pin1, pin2 = self.state.last_event_pins
            else:
                pin1, pin2 = (0, 0)

        leds = self.state.compute_leds()
        body = bytes([pin1 & 0xFF, pin2 & 0xFF] + [x & 0xFF for x in leds])
        frame_wo_crc = bytes([PROTOCOL_SOF, C2A_LENGTH]) + body
        crc = calc_checksum(frame_wo_crc)
        frame = frame_wo_crc + bytes([crc])

        with self._tx_lock:
            try:
                self.ser.write(frame)
                if self.verbose:
                    print(f"[TX] {frame.hex(' ')}", file=sys.stderr)
            except Exception as e:
                if self.verbose:
                    print(f"[ERR] TX failed: {e}", file=sys.stderr)

    # --- Handling incoming A2C packets ---
    def _on_a2c_packet(self, pkt: A2C_Packet):
        if self.verbose:
            print(f"[PKT] cmd=0x{pkt.command:02X} pin1={pkt.pin1} pin2={pkt.pin2}", file=sys.stderr)

        if pkt.command == CMD_BTN_PRESSED:
            self._handle_press(pkt.pin1, pkt.pin2)
        elif pkt.command == CMD_BTN_RELEASED:
            self._handle_release(pkt.pin1, pkt.pin2)
        elif pkt.command == CMD_MODE_CHECK_KEYBOARD:
            self.state.set_mode_check(True)
            # сбрасываем «вспышку», чтобы следующие пины были видны
            self._last_event_time = time.perf_counter() - (self._check_hold_ms / 1000.0) - 1
        elif pkt.command == CMD_MODE_RUN:
            self.state.set_mode_check(False)
        else:
            if self.verbose:
                print(f"[WARN] Unknown command 0x{pkt.command:02X}", file=sys.stderr)

        # Отправим статус сразу после команды (не дожидаясь таймера)
        self._send_status()

    def _handle_press(self, p1: int, p2: int):
        if not (1 <= p1 <= 15 and 1 <= p2 <= 15) or p1 == p2:
            if self.verbose:
                print(f"[WARN] Ignoring bad press ({p1},{p2})", file=sys.stderr)
            return
        self.state.press(p1, p2)
        self._last_event_time = time.perf_counter()

    def _handle_release(self, p1: int, p2: int):
        if not (1 <= p1 <= 15 and 1 <= p2 <= 15) or p1 == p2:
            if self.verbose:
                print(f"[WARN] Ignoring bad release ({p1},{p2})", file=sys.stderr)
            return
        self.state.release(p1, p2)
        self._last_event_time = time.perf_counter()


# --- Вспомогательные утилиты для тестов (необязательно) ---
def build_a2c(command: int, p1: int, p2: int) -> bytes:
    """
    Сборка пакета App->Ctrl для локальных тестов:
    SOF | Length(=4) | Cmd | Pin1 | Pin2 | Checksum
    """
    frame_wo_crc = bytes([PROTOCOL_SOF, A2C_LENGTH, command & 0xFF, p1 & 0xFF, p2 & 0xFF])
    crc = calc_checksum(frame_wo_crc)
    return frame_wo_crc + bytes([crc])


def main():
    ap = argparse.ArgumentParser(description="Keyboard Controller Emulator (serial)")
    ap.add_argument("--port", required=True, help="Serial port (e.g., COM7 or /dev/ttyUSB0)")
    ap.add_argument("--baud", type=int, default=115200, help="Baud rate")
    ap.add_argument("--period", type=float, default=2.0, help="Status period in seconds (default 2.0 = 0.5 Hz)")
    ap.add_argument("-v", "--verbose", action="store_true", help="Verbose logs to stderr")
    args = ap.parse_args()

    emu = ControllerEmulator(args.port, args.baud, period_s=args.period, verbose=args.verbose)
    emu.open()
    emu.run()


if __name__ == "__main__":
    main()
