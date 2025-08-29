# controller_emulator.py
import argparse
import sys
import threading
import time
from dataclasses import dataclass, field
from typing import List, Tuple, Optional

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
CMD_MODE_CONFIGURE        = 0x05   # новое

# Длины «payload+checksum» (байты после поля Length и ВКЛЮЧАЯ checksum)
A2C_LENGTH = 4                                  # Cmd(1)+P1(1)+P2(1)+Chk(1)
C2A_LENGTH = 18                                 # P1(1)+P2(1)+LED+Chk(1)

def calc_checksum(data: bytes) -> int:
    return sum(data) & 0xFF


@dataclass
class A2C_Packet:
    command: int
    pin1: int
    pin2: int


@dataclass
class ControllerState:
    # режимы
    mode: int = CMD_MODE_RUN  # по умолчанию RUN
    # «нажатые» пары для RUN (не шлём наружу, но держим состояние)
    pressed_pairs: set = field(default_factory=set)

    # индекс перебора для CHECK
    check_index: int = 1

    def set_mode(self, mode: int):
        self.mode = mode
        # сбросить служебные состояния при смене режима
        if mode == CMD_MODE_CHECK_KEYBOARD:
            self.check_index = 1

    # RUN: фиксация нажатий
    def press(self, p1: int, p2: int):
        self.pressed_pairs.add(self._norm_pair(p1, p2))

    def release(self, p1: int, p2: int):
        self.pressed_pairs.discard(self._norm_pair(p1, p2))

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

    def feed(self, chunk: bytes):
        self.buf.extend(chunk)
        self._drain()

    def _drain(self):
        while True:
            sof_index = self.buf.find(bytes([PROTOCOL_SOF]))
            if sof_index == -1:
                self.buf.clear()
                return
            if sof_index > 0:
                del self.buf[:sof_index]

            if len(self.buf) < 2:
                return

            length = self.buf[1]
            expected = 1 + 1 + length  # SOF + Length + (payload+checksum)
            if len(self.buf) < expected:
                return

            frame = bytes(self.buf[:expected])
            del self.buf[:expected]

            # Проверка длины
            if length != A2C_LENGTH:
                if self.verbose:
                    print(f"[WARN] Bad A2C length={length}, expected {A2C_LENGTH}", file=sys.stderr)
                continue

            # Проверка checksum (по всем байтам до checksum)
            checksum_expected = frame[-1]
            checksum_calc = calc_checksum(frame[:-1])
            if checksum_expected != checksum_calc:
                if self.verbose:
                    print(f"[WARN] Bad checksum: got=0x{checksum_expected:02X}, calc=0x{checksum_calc:02X}", file=sys.stderr)
                continue

            cmd  = frame[2]
            pin1 = frame[3]
            pin2 = frame[4]
            self.on_packet(A2C_Packet(cmd, pin1, pin2))


class ControllerEmulator:
    def __init__(self, port: str, baud: int, check_interval_s: float = 0.2, verbose: bool = False):
        if serial is None:
            raise RuntimeError("pyserial не установлен. Установите: pip install pyserial")

        self.port_name = port
        self.baud = baud
        self.verbose = verbose

        self.ser: Optional[serial.Serial] = None
        self.framer = Framer(self._on_a2c_packet, verbose=verbose)
        self.state = ControllerState()
        self._stop = threading.Event()
        self._tx_lock = threading.Lock()

        # Таймер для CHECK режима
        self.check_interval_s = max(0.02, check_interval_s)  # защитимся от 0
        self._next_check_time = time.perf_counter()

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
        while not self._stop.is_set():
            now = time.perf_counter()

            # Отправка только в CHECK режиме
            if self.state.mode == CMD_MODE_CHECK_KEYBOARD and now >= self._next_check_time:
                self._send_check_status()
                self._next_check_time = now + self.check_interval_s

            time.sleep(0.001)

    # --- Формирование Ctrl->App пакетов ---
    def _send_check_status(self):
        """
        CHECK: перебор pin/LED. На каждом шаге:
        pin1 = i (1..15), pin2 = 0
        leds[i-1] = 1, остальные = 0
        """
        if not self.ser:
            return

        i = self.state.check_index
        if i < 1 or i > 15:
            i = 1
        self.state.check_index = 1 + (i % 15)  # 1..15 по кругу

        pin1, pin2 = i, i
        leds = [0]*15
        leds[i-1] = 1

        body = bytes([pin1 & 0xFF, pin2 & 0xFF] + leds)
        frame_wo_crc = bytes([PROTOCOL_SOF, C2A_LENGTH]) + body
        crc = calc_checksum(frame_wo_crc)
        frame = frame_wo_crc + bytes([crc])

        with self._tx_lock:
            try:
                self.ser.write(frame)
                if self.verbose:
                    print(f"[TX][CHECK] idx={i} -> {frame.hex(' ')}", file=sys.stderr)
            except Exception as e:
                if self.verbose:
                    print(f"[ERR] TX failed: {e}", file=sys.stderr)

    # --- Обработка входящих A2C ---
    def _on_a2c_packet(self, pkt: A2C_Packet):
        # Переключение режимов
        if pkt.command == CMD_MODE_CONFIGURE:
            if self.verbose and self.state.mode != CMD_MODE_CONFIGURE:
                print("[MODE] CONFIGURE", file=sys.stderr)
            self.state.set_mode(CMD_MODE_CONFIGURE)
            return

        if pkt.command == CMD_MODE_RUN:
            if self.verbose and self.state.mode != CMD_MODE_RUN:
                print("[MODE] RUN", file=sys.stderr)
            self.state.set_mode(CMD_MODE_RUN)
            return

        if pkt.command == CMD_MODE_CHECK_KEYBOARD:
            if self.verbose and self.state.mode != CMD_MODE_CHECK_KEYBOARD:
                print("[MODE] CHECK_KEYBOARD", file=sys.stderr)
            self.state.set_mode(CMD_MODE_CHECK_KEYBOARD)
            # Сразу сдвинем таймер, чтобы немедленно отправить первый шаг
            self._next_check_time = 0.0
            return

        # Поведение по режимам
        if self.state.mode == CMD_MODE_CONFIGURE:
            # В конфиге: ничего не шлём и считаем любые иные команды ошибкой
            if self.verbose:
                print(f"[ERROR][CONFIGURE] Unexpected command 0x{pkt.command:02X} (ignored)", file=sys.stderr)
            return

        if self.state.mode == CMD_MODE_RUN:
            # В RUN: принимаем только нажатия/отпускания; ничего не отправляем
            if pkt.command == CMD_BTN_PRESSED:
                self._handle_press(pkt.pin1, pkt.pin2)
            elif pkt.command == CMD_BTN_RELEASED:
                self._handle_release(pkt.pin1, pkt.pin2)
            else:
                if self.verbose:
                    print(f"[WARN][RUN] Ignoring non-press/release cmd 0x{pkt.command:02X}", file=sys.stderr)
            return

        if self.state.mode == CMD_MODE_CHECK_KEYBOARD:
            # В CHECK входящие команды (кроме смены режима) игнорируем
            if self.verbose:
                print(f"[INFO][CHECK] Ignored cmd 0x{pkt.command:02X}", file=sys.stderr)
            return

    def _handle_press(self, p1: int, p2: int):
        if not (1 <= p1 <= 15 and 1 <= p2 <= 15) or p1 == p2:
            if self.verbose:
                print(f"[WARN] Ignoring bad press ({p1},{p2})", file=sys.stderr)
            return
        self.state.press(p1, p2)
        if self.verbose:
            print(f"[PKT][RUN] press {p1}-{p2}", file=sys.stderr)

    def _handle_release(self, p1: int, p2: int):
        if not (1 <= p1 <= 15 and 1 <= p2 <= 15) or p1 == p2:
            if self.verbose:
                print(f"[WARN] Ignoring bad release ({p1},{p2})", file=sys.stderr)
            return
        self.state.release(p1, p2)
        if self.verbose:
            print(f"[PKT][RUN] release {p1}-{p2}", file=sys.stderr)


# --- Вспомогательные утилиты (локальный тест без UART) ---
def build_a2c(command: int, p1: int = 0, p2: int = 0) -> bytes:
    """
    SOF | Length(=4) | Cmd | Pin1 | Pin2 | Checksum
    """
    frame_wo_crc = bytes([PROTOCOL_SOF, A2C_LENGTH, command & 0xFF, p1 & 0xFF, p2 & 0xFF])
    crc = calc_checksum(frame_wo_crc)
    return frame_wo_crc + bytes([crc])


def main():
    ap = argparse.ArgumentParser(description="Keyboard Controller Emulator (serial)")
    ap.add_argument("--port", required=True, help="Serial port (e.g., COM7 or /dev/ttyUSB0)")
    ap.add_argument("--baud", type=int, default=115200, help="Baud rate")
    ap.add_argument("--check-interval", type=float, default=2.0,
                    help="Interval (s) between steps in CHECK mode (default 2.0)")
    ap.add_argument("-v", "--verbose", action="store_true", help="Verbose logs to stderr")
    args = ap.parse_args()

    emu = ControllerEmulator(args.port, args.baud, check_interval_s=args["check_interval"] if isinstance(args, dict) else args.check_interval, verbose=args.verbose)
    emu.open()
    emu.run()


if __name__ == "__main__":
    main()
