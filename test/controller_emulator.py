# controller_emulator.py
import argparse
import sys
import threading
import time
from dataclasses import dataclass, field
from typing import List, Tuple, Optional, Set

try:
    import serial  # pyserial
except ImportError:
    serial = None


# --- Протокол ---
PROTOCOL_SOF = 0xAA

# Команды App -> Ctrl (обновлено)
CMD_ECHO                  = 0x01
CMD_BTN_PRESSED           = 0x02
CMD_BTN_RELEASED          = 0x03
CMD_MODE_CHECK_KEYBOARD   = 0x04
CMD_MODE_RUN              = 0x05
CMD_MODE_CONFIGURE        = 0x06
CMD_MODE_DIODE_CONFIG     = 0x07
CMD_MODE_DIODE_CONFIG_DEL = 0x08
CMD_MODE_DIODE_CLEAR      = 0x09
CMD_DIODE_PRESSED         = 0x0A
CMD_DIODE_RELEASED        = 0x0B

# App->Ctrl кадр: SOF | Length(=4) | Cmd | Pin1 | Pin2 | Checksum
A2C_LENGTH = 4


def calc_checksum(data: bytes) -> int:
    """Сумма всех байт по модулю 256 (включая SOF и Length, без последнего checksum-байта)."""
    return sum(data) & 0xFF


@dataclass
class A2C_Packet:
    command: int
    pin1: int
    pin2: int


@dataclass
class ControllerState:
    """mode — активный режим. В RUN ничего не храним. В CHECK крутится индекс."""
    mode: int = CMD_MODE_RUN
    pressed_pairs: Set[Tuple[int, int]] = field(default_factory=set)
    check_index: int = 1

    def set_mode(self, mode: int):
        self.mode = mode
        if mode == CMD_MODE_CHECK_KEYBOARD:
            self.check_index = 1  # стартуем с 1


class Framer:
    """
    Разбор App->Ctrl:
    SOF(0xAA) | Length | Command | Pin1 | Pin2 | Checksum
    """
    def __init__(self, on_packet, verbose: bool = False):
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

            if length != A2C_LENGTH:
                if self.verbose:
                    print(f"[WARN] Bad A2C length={length}, expected {A2C_LENGTH}", file=sys.stderr)
                continue

            checksum_expected = frame[-1]
            checksum_calc = calc_checksum(frame[:-1])
            if checksum_expected != checksum_calc:
                if self.verbose:
                    print(f"[WARN] Bad checksum: got=0x{checksum_expected:02X}, "
                          f"calc=0x{checksum_calc:02X}", file=sys.stderr)
                continue

            cmd  = frame[2]
            pin1 = frame[3]
            pin2 = frame[4]
            self.on_packet(A2C_Packet(cmd, pin1, pin2))


class ControllerEmulator:
    """
    RUN (0x05):
      - BTN_PRESSED(0x02,p1,p2) -> немедленно Ctrl->App: pin1=p1, pin2=p2, leds_num=1, leds=(p1,p2)
      - BTN_RELEASED(0x03) -> только лог.

    CHECK (0x04):
      - Самогенерация по кругу i=1..15 каждые --check-interval секунд:
        pin1=i, pin2=i, leds_num=0.
      - BTN_PRESSED(0x02) из приложения = "нажатие на диод" -> отвечаем пакетом с leds_num=1, leds=(p1,p2).
      - BTN_RELEASED(0x03) -> только лог.
      - DIODE_PRESSED/RELEASED (0x0A/0x0B) -> только печатаем пины (без ответа).

    DIODE_CONFIG (0x07), DIODE_CONFIG_DEL (0x08), DIODE_CLEAR (0x09):
      - 0x07 и 0x08: вход в режим и немедленный нулевой ACK (pin1=0, pin2=0, leds_num=0).
      - 0x09: лог "принято" и немедленный нулевой ACK.

    ECHO (0x01):
      - Всегда отвечаем нулевым пакетом во всех режимах.
    """
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

        self.check_interval_s = max(0.02, check_interval_s)
        self._next_check_time = time.perf_counter()

    # --- Lifecycle ---
    def open(self):
        self.ser = serial.Serial(
            self.port_name,
            self.baud,
            timeout=0,          # неблокирующее чтение
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
            if self.state.mode == CMD_MODE_CHECK_KEYBOARD:
                now = time.perf_counter()
                if now >= self._next_check_time:
                    self._send_check_step()
                    self._next_check_time = now + self.check_interval_s
            time.sleep(0.001)

    # --- Ctrl->App packet builder ---
    @staticmethod
    def _build_ctrl2app(pin1: int, pin2: int, led_pairs: List[Tuple[int, int]]) -> bytes:
        """
        Ctrl->App:
        SOF | Length | pin1 | pin2 | leds_num | (a1 k1) ... (aN kN) | checksum
        Length = 4 + 2*N
        """
        leds_num = len(led_pairs)
        payload = bytes([pin1 & 0xFF, pin2 & 0xFF, leds_num & 0xFF])

        if leds_num:
            flat: List[int] = []
            for a, k in led_pairs:
                flat.append(a & 0xFF)
                flat.append(k & 0xFF)
            payload += bytes(flat)

        length = 4 + 2 * leds_num
        frame_wo_crc = bytes([PROTOCOL_SOF, length]) + payload
        crc = calc_checksum(frame_wo_crc)
        return frame_wo_crc + bytes([crc])

    # --- TX helpers ---
    def _send_zero_ack(self, tag: str = "ACK-ZERO"):
        """Нулевой пакет (pin1=0, pin2=0, leds_num=0)."""
        if not self.ser:
            return
        frame = self._build_ctrl2app(0, 0, [])
        with self._tx_lock:
            try:
                self.ser.write(frame)
                if self.verbose:
                    print(f"[TX][{tag}] {frame.hex(' ')}", file=sys.stderr)
            except Exception as e:
                if self.verbose:
                    print(f"[ERR] TX failed: {e}", file=sys.stderr)

    def _send_run_update(self, p1: int, p2: int):
        """RUN: немедленный ответ с одной LED-парой (p1,p2)."""
        if not self.ser:
            return
        frame = self._build_ctrl2app(p1, p2, [(p1, p2)])
        with self._tx_lock:
            try:
                self.ser.write(frame)
                if self.verbose:
                    print(f"[TX][RUN] {frame.hex(' ')}", file=sys.stderr)
            except Exception as e:
                if self.verbose:
                    print(f"[ERR] TX failed: {e}", file=sys.stderr)

    def _send_check_step(self):
        """CHECK: шлём pin1=i, pin2=i, leds_num=0; i=1..15 по кругу."""
        if not self.ser:
            return
        i = self.state.check_index
        if i < 1 or i > 15:
            i = 1
        frame = self._build_ctrl2app(i, i, [])
        self.state.check_index = 1 + (i % 15)
        with self._tx_lock:
            try:
                self.ser.write(frame)
                if self.verbose:
                    print(f"[TX][CHECK] i={i} -> {frame.hex(' ')}", file=sys.stderr)
            except Exception as e:
                if self.verbose:
                    print(f"[ERR] TX failed: {e}", file=sys.stderr)

    def _send_check_led_press(self, p1: int, p2: int):
        """CHECK: ответ на BTN_PRESSED из приложения — одна LED-пара (p1,p2)."""
        if not self.ser:
            return
        frame = self._build_ctrl2app(p1, p2, [(p1, p2)])
        with self._tx_lock:
            try:
                self.ser.write(frame)
                if self.verbose:
                    print(f"[TX][CHECK][LED] press {p1}-{p2} -> {frame.hex(' ')}", file=sys.stderr)
            except Exception as e:
                if self.verbose:
                    print(f"[ERR] TX failed: {e}", file=sys.stderr)

    # --- Обработка входящих пакетов ---
    def _on_a2c_packet(self, pkt: A2C_Packet):
        cmd = pkt.command

        # Команды, на которые отвечаем независимо от режима
        if cmd == CMD_ECHO:
            if self.verbose:
                print("[ECHO] request -> reply with empty status", file=sys.stderr)
            self._send_zero_ack(tag="ECHO")
            return

        if cmd == CMD_MODE_DIODE_CLEAR:
            if self.verbose:
                print("[DIODE_CLEAR] accepted -> reply with empty status", file=sys.stderr)
            self._send_zero_ack(tag="DIODE_CLEAR")
            return

        if cmd == CMD_DIODE_PRESSED:
            if self.verbose:
                print(f"[DIODE] pressed {pkt.pin1}-{pkt.pin2}", file=sys.stderr)
            return

        if cmd == CMD_DIODE_RELEASED:
            if self.verbose:
                print(f"[DIODE] released {pkt.pin1}-{pkt.pin2}", file=sys.stderr)
            return

        # Переключение режимов
        if cmd == CMD_MODE_RUN:
            if self.verbose and self.state.mode != CMD_MODE_RUN:
                print("[MODE] RUN", file=sys.stderr)
            self.state.set_mode(CMD_MODE_RUN)
            return

        if cmd == CMD_MODE_CHECK_KEYBOARD:
            if self.verbose and self.state.mode != CMD_MODE_CHECK_KEYBOARD:
                print("[MODE] CHECK_KEYBOARD", file=sys.stderr)
            self.state.set_mode(CMD_MODE_CHECK_KEYBOARD)
            self._next_check_time = 0.0  # форсим первый шаг
            return

        if cmd == CMD_MODE_CONFIGURE:
            if self.verbose and self.state.mode != CMD_MODE_CONFIGURE:
                print("[MODE] CONFIGURE (stub)", file=sys.stderr)
            self.state.set_mode(CMD_MODE_CONFIGURE)
            return

        if cmd == CMD_MODE_DIODE_CONFIG:
            if self.verbose and self.state.mode != CMD_MODE_DIODE_CONFIG:
                print("[MODE] DIODE_CONFIG", file=sys.stderr)
            self.state.set_mode(CMD_MODE_DIODE_CONFIG)
            self._send_zero_ack(tag="DIODE_CONFIG")
            return

        if cmd == CMD_MODE_DIODE_CONFIG_DEL:
            if self.verbose and self.state.mode != CMD_MODE_DIODE_CONFIG_DEL:
                print("[MODE] DIODE_CONFIG_DEL", file=sys.stderr)
            self.state.set_mode(CMD_MODE_DIODE_CONFIG_DEL)
            self._send_zero_ack(tag="DIODE_CONFIG_DEL")
            return

        # Поведение по активным режимам
        if self.state.mode == CMD_MODE_RUN:
            if cmd == CMD_BTN_PRESSED:
                p1, p2 = pkt.pin1, pkt.pin2
                if 1 <= p1 <= 15 and 1 <= p2 <= 15:
                    if self.verbose:
                        print(f"[RUN] press {p1}-{p2}", file=sys.stderr)
                    self._send_run_update(p1, p2)
                else:
                    if self.verbose:
                        print(f"[RUN][WARN] press out of range: {p1}-{p2}", file=sys.stderr)
                return
            if cmd == CMD_BTN_RELEASED:
                if self.verbose:
                    print(f"[RUN] release {pkt.pin1}-{pkt.pin2}", file=sys.stderr)
                return
            return  # прочее игнорируем

        if self.state.mode == CMD_MODE_CHECK_KEYBOARD:
            # BTN_* в CHECK используем для эхо-нажатия диода (как ты просил ранее)
            if cmd == CMD_BTN_PRESSED:
                p1, p2 = pkt.pin1, pkt.pin2
                if 1 <= p1 <= 15 and 1 <= p2 <= 15:
                    if self.verbose:
                        print(f"[CHECK] LED press from app: {p1}-{p2}", file=sys.stderr)
                    self._send_check_led_press(p1, p2)
                else:
                    if self.verbose:
                        print(f"[CHECK][WARN] LED press out of range: {p1}-{p2}", file=sys.stderr)
                return
            if cmd == CMD_BTN_RELEASED:
                if self.verbose:
                    print(f"[CHECK] LED release from app: {pkt.pin1}-{pkt.pin2}", file=sys.stderr)
                return
            return  # прочее игнорируем

        # Остальное — без ответа, только лог при verbose
        if self.verbose:
            print(f"[INFO] Cmd 0x{cmd:02X} ignored in mode 0x{self.state.mode:02X}", file=sys.stderr)


# --- Helpers ---
def build_a2c(command: int, p1: int = 0, p2: int = 0) -> bytes:
    """App->Ctrl кадр: SOF | Length(=4) | Cmd | Pin1 | Pin2 | Checksum"""
    frame_wo_crc = bytes([PROTOCOL_SOF, A2C_LENGTH, command & 0xFF, p1 & 0xFF, p2 & 0xFF])
    crc = calc_checksum(frame_wo_crc)
    return frame_wo_crc + bytes([crc])


def main():
    ap = argparse.ArgumentParser(description="Keyboard Controller Emulator (serial)")
    ap.add_argument("--port", required=True, help="Serial port (e.g., COM7 or /dev/ttyUSB0)")
    ap.add_argument("--baud", type=int, default=115200, help="Baud rate")
    ap.add_argument("--check-interval", type=float, default=2.0,
                    help="Period (s) for CHECK mode step")
    ap.add_argument("-v", "--verbose", action="store_true", help="Verbose logs to stderr")
    args = ap.parse_args()

    emu = ControllerEmulator(
        args.port,
        args.baud,
        check_interval_s=args.check_interval,
        verbose=args.verbose
    )
    try:
        emu.open()
        emu.run()
    except Exception as e:
        print(f"[ERROR] {e}", file=sys.stderr)


if __name__ == "__main__":
    main()
