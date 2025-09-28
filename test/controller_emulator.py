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

# Команды App -> Ctrl
CMD_BTN_PRESSED           = 0x01
CMD_BTN_RELEASED          = 0x02
CMD_MODE_CHECK_KEYBOARD   = 0x03
CMD_MODE_RUN              = 0x04
CMD_MODE_CONFIGURE        = 0x05
CMD_MODE_DIODE_CONFIG     = 0x06
CMD_MODE_DIODE_CONFIG_DEL = 0x07

# App->Ctrl кадр: SOF | Length(=4) | Cmd | Pin1 | Pin2 | Checksum
A2C_LENGTH = 4


def calc_checksum(data: bytes) -> int:
    """
    Контрольная сумма — сумма всех байт по модулю 256.
    Важно: для Ctrl->App по ТЗ "over all previous bytes" — включая SOF и Length.
    Для App->Ctrl здесь делаем так же (кадр проверяем целиком без последнего байта).
    """
    return sum(data) & 0xFF


@dataclass
class A2C_Packet:
    command: int
    pin1: int
    pin2: int


@dataclass
class ControllerState:
    """
    В RUN по требованиям мы не храним состояния. Поле mode — только для переключений.
    """
    mode: int = CMD_MODE_RUN
    # оставлено для будущих режимов; в RUN не используется
    pressed_pairs: Set[Tuple[int, int]] = field(default_factory=set)

    def set_mode(self, mode: int):
        self.mode = mode

    @staticmethod
    def norm_pair(a: int, b: int) -> Tuple[int, int]:
        return (min(a, b), max(a, b))


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
    Реализован режим RUN (0x04) по твоим правилам:
      - При BTN_PRESSED(p1,p2) немедленно отправляем Ctrl->App пакет:
          pin1=p1, pin2=p2, leds_num=1, leds[0]=(p1,p2)
      - BTN_RELEASED — только печать, без отправки.
      - Диапазон пинов 1..15; p1 == p2 допустим.
      - Никаких состояний не храним.
    Остальные режимы и команды — подключены как заглушки без Tx, чтобы не мешать тесту.
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

        # зарезервировано «на будущее» (в RUN не используется)
        self.check_interval_s = max(0.02, check_interval_s)

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
        # В RUN ничего периодически не отправляем
        while not self._stop.is_set():
            time.sleep(0.01)

    # --- Ctrl->App packet builder ---
    @staticmethod
    def _build_ctrl2app(pin1: int, pin2: int, led_pairs: List[Tuple[int, int]]) -> bytes:
        """
        Ctrl->App:
        SOF | Length | pin1 | pin2 | leds_num | (a1 k1) ... (aN kN) | checksum
        Length = 4 + 2*N   (pin1,pin2,leds_num, 2*N, checksum)
        Checksum = sum(SOF..последний байт до checksum) & 0xFF
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

    # --- Обработка входящих пакетов ---
    def _on_a2c_packet(self, pkt: A2C_Packet):
        cmd = pkt.command

        # Переключение режимов (без Tx, кроме RUN-логики)
        if cmd == CMD_MODE_RUN:
            if self.verbose and self.state.mode != CMD_MODE_RUN:
                print("[MODE] RUN", file=sys.stderr)
            self.state.set_mode(CMD_MODE_RUN)
            return

        if cmd == CMD_MODE_CHECK_KEYBOARD:
            if self.verbose:
                print("[MODE] CHECK_KEYBOARD (stub, no TX)", file=sys.stderr)
            self.state.set_mode(CMD_MODE_CHECK_KEYBOARD)
            return

        if cmd == CMD_MODE_CONFIGURE:
            if self.verbose:
                print("[MODE] CONFIGURE (stub, no TX)", file=sys.stderr)
            self.state.set_mode(CMD_MODE_CONFIGURE)
            return

        if cmd == CMD_MODE_DIODE_CONFIG:
            if self.verbose:
                print("[MODE] DIODE_CONFIG (stub, no TX here for RUN testing)", file=sys.stderr)
            self.state.set_mode(CMD_MODE_DIODE_CONFIG)
            # По твоему финальному ТЗ ACK для 0x06/0x07 будет добавлен на этапе этих режимов.
            return

        if cmd == CMD_MODE_DIODE_CONFIG_DEL:
            if self.verbose:
                print("[MODE] DIODE_CONFIG_DEL (stub, no TX here for RUN testing)", file=sys.stderr)
            self.state.set_mode(CMD_MODE_DIODE_CONFIG_DEL)
            return

        # RUN-логика ответа
        if self.state.mode == CMD_MODE_RUN:
            if cmd == CMD_BTN_PRESSED:
                p1, p2 = pkt.pin1, pkt.pin2
                # Диапазон 1..15, p1==p2 допустим — просто отправляем пакет
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
                # Ничего не отправляем
                return

            # Остальные команды в RUN — тихо игнорируем
            if self.verbose:
                print(f"[RUN][INFO] ignore cmd 0x{cmd:02X}", file=sys.stderr)
            return

        # В других режимах на press/release ничего не делаем (стабильно «тихо»)
        if cmd in (CMD_BTN_PRESSED, CMD_BTN_RELEASED) and self.verbose:
            print(f"[INFO] press/release ignored in mode 0x{self.state.mode:02X}", file=sys.stderr)


# --- Helpers ---
def build_a2c(command: int, p1: int = 0, p2: int = 0) -> bytes:
    """
    Построить App->Ctrl кадр:
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
                    help="Reserved for future CHECK mode periodicity")
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
