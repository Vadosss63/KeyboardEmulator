import argparse
import sys
import threading
import time

from serial_port import SerialPort

from protocol import (
    CMD_ECHO,
    CMD_BTN_PRESSED,
    CMD_BTN_RELEASED,
    CMD_MODE_CHECK_KEYBOARD,
    CMD_MODE_RUN,
    CMD_MODE_CONFIGURE,
    CMD_MODE_DIODE_CONFIG,
    CMD_MODE_DIODE_CONFIG_DEL,
    CMD_MODE_DIODE_CLEAR,
    CMD_DIODE_PRESSED,
    CMD_DIODE_RELEASED,
    Packet,         
    Framer,
    build_cmd_no_payload,
)

from states import (
    BaseState,
    RunState,
    CheckKeyboardState,
    ConfigureState,
    DiodeConfigState,
    DiodeConfigDelState,
)


class ControllerEmulator:
    def __init__(
        self,
        port: str,
        baud: int,
        check_interval_s: float = 0.2,
        verbose: bool = False,
        io: SerialPort | None = None,
    ):
        self.port_name = port
        self.baud = baud
        self.verbose = verbose

        self.io: SerialPort = io or SerialPort(
            port,
            baud,
            timeout=0.0,
            write_timeout=1.0,
            verbose=verbose,
        )

        self.framer = Framer(self._on_packet, verbose=verbose)
        self._stop = threading.Event()
        self._tx_lock = threading.Lock()

        self.check_interval_s = max(0.02, check_interval_s)

        self.state: BaseState = RunState()

    # --- lifecycle ---

    def open(self):
        self.io.open()
        if self.verbose:
            print(f"[INFO] Opened {self.port_name} @ {self.baud}", file=sys.stderr)

    def close(self):
        self.io.close()

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

    def _set_state(self, new_state: BaseState):
        prev = self.state
        self.state = new_state
        self.state.on_enter(prev, self)

    def _rx_pump(self):
        if not self.io.is_open:
            return
        data = self.io.read(1024)
        if data:
            if self.verbose:
                print(f"[RX] {data.hex(' ')}", file=sys.stderr)
            self.framer.feed(data)

    def _tx_loop(self):
        while not self._stop.is_set():
            now = time.perf_counter()
            self.state.tick(now, self)
            time.sleep(0.001)

    def _send_frame(self, frame: bytes, tag: str = ""):
        if not self.io.is_open:
            return
        with self._tx_lock:
            try:
                self.io.write(frame)
                if self.verbose:
                    print(f"[TX]{'[' + tag + ']' if tag else ''} {frame.hex(' ')}", file=sys.stderr)
            except Exception as e:
                if self.verbose:
                    print(f"[ERR] TX failed: {e}", file=sys.stderr)

    def _send_zero_ack(self, cmd: int, tag: str = "ACK-ZERO"):
        frame = build_cmd_no_payload(cmd)
        self._send_frame(frame, tag=tag)

    def _on_packet(self, pkt: Packet):
        cmd = pkt.command

        if cmd == CMD_ECHO:
            if self.verbose:
                print("[ECHO] request -> reply with empty status", file=sys.stderr)
            self._send_zero_ack(CMD_ECHO, tag="ECHO")
            return

        if cmd == CMD_MODE_DIODE_CLEAR:
            if self.verbose:
                print("[DIODE_CLEAR] accepted -> reply with empty status", file=sys.stderr)
            self._send_zero_ack(CMD_MODE_DIODE_CLEAR, tag="DIODE_CLEAR")
            return

        if cmd == CMD_DIODE_PRESSED:
            if self.verbose:
                print(f"[DIODE] pressed {pkt.pin1}-{pkt.pin2}", file=sys.stderr)
            return

        if cmd == CMD_DIODE_RELEASED:
            if self.verbose:
                print(f"[DIODE] released {pkt.pin1}-{pkt.pin2}", file=sys.stderr)
            return


        if cmd == CMD_MODE_RUN:
            self._set_state(RunState())
            return

        if cmd == CMD_MODE_CHECK_KEYBOARD:
            self._set_state(CheckKeyboardState())
            return

        if cmd == CMD_MODE_CONFIGURE:
            self._set_state(ConfigureState())
            return

        if cmd == CMD_MODE_DIODE_CONFIG:
            self._set_state(DiodeConfigState())
            self._send_zero_ack(CMD_MODE_DIODE_CONFIG, tag="DIODE_CONFIG")
            return

        if cmd == CMD_MODE_DIODE_CONFIG_DEL:
            self._set_state(DiodeConfigDelState())
            self._send_zero_ack(CMD_MODE_DIODE_CONFIG_DEL, tag="DIODE_CONFIG_DEL")
            return

        self.state.handle_packet(pkt, self)


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
        verbose=args.verbose,
    )
    try:
        emu.open()
        emu.run()
    except Exception as e:
        print(f"[ERROR] {e}", file=sys.stderr)


if __name__ == "__main__":
    main()
