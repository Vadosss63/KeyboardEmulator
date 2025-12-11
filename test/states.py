from __future__ import annotations

from dataclasses import dataclass
import time

from protocol import (
    CMD_MODE_RUN,
    CMD_MODE_CHECK_KEYBOARD,
    CMD_MODE_CONFIGURE,
    CMD_MODE_DIODE_CONFIG,
    CMD_MODE_DIODE_CONFIG_DEL,
    CMD_BTN_PRESSED,
    CMD_BTN_RELEASED,
    Packet,
    build_status,
)


class BaseState:
    mode_cmd: int = 0
    name: str = "BASE"

    def on_enter(self, prev_state: "BaseState | None", emu) -> None:
        if emu.verbose:
            import sys
            prev_name = prev_state.name if prev_state else "-"
            print(f"[MODE] {prev_name} -> {self.name}", file=sys.stderr)

    def handle_packet(self, pkt: Packet, emu)  -> None:
        if emu.verbose:
            import sys
            print(f"[{self.name}] ignore cmd=0x{pkt.command:02X}", file=sys.stderr)

    def tick(self, now: float, emu) -> None:
        return


# --- RUN ----------------------------------------------------------------------


class RunState(BaseState):
    mode_cmd = CMD_MODE_RUN
    name = "RUN"

    def handle_packet(self, pkt: Packet, emu)  -> None:
        cmd = pkt.command

        if cmd == CMD_BTN_PRESSED:
            p1, p2 = pkt.pin1, pkt.pin2
            if 1 <= p1 <= 15 and 1 <= p2 <= 15:
                if emu.verbose:
                    import sys
                    print(f"[RUN] press {p1}-{p2}", file=sys.stderr)

                frame = build_status(p1, p2, [(p1, p2)])
                emu._send_frame(frame, tag="RUN")
            else:
                if emu.verbose:
                    import sys
                    print(f"[RUN][WARN] press out of range {p1}-{p2}", file=sys.stderr)
            return

        if cmd == CMD_BTN_RELEASED:
            if emu.verbose:
                import sys
                print(f"[RUN] release {pkt.pin1}-{pkt.pin2}", file=sys.stderr)
            return

        super().handle_packet(pkt, emu)


# --- CHECK_KEYBOARD -----------------------------------------------------------


@dataclass
class CheckKeyboardState(BaseState):
    mode_cmd: int = CMD_MODE_CHECK_KEYBOARD
    name: str = "CHECK_KEYBOARD"

    _index: int = 1
    _next_step_at: float = 0.0

    def on_enter(self, prev_state: BaseState | None, emu) -> None:
        super().on_enter(prev_state, emu)
        now = time.perf_counter()
        self._index = 1
        self._next_step_at = now

    def tick(self, now: float, emu) -> None:
        """Every emu.check_interval_s we send pin1=i, pin2=i, leds_num=0, i=1..15 in a loop."""
        if now < self._next_step_at:
            return

        i = self._index
        if i < 1 or i > 15:
            i = 1

        frame = build_status(i, i, [])
        emu._send_frame(frame, tag=f"CHECK i={i}")

        self._index = 1 + (i % 15)
        self._next_step_at = now + emu.check_interval_s

    def handle_packet(self, pkt: Packet, emu)  -> None:
        cmd = pkt.command

        if cmd == CMD_BTN_PRESSED:
            p1, p2 = pkt.pin1, pkt.pin2
            if 1 <= p1 <= 15 and 1 <= p2 <= 15:
                if emu.verbose:
                    import sys
                    print(f"[CHECK] LED press from app: {p1}-{p2}", file=sys.stderr)
                frame = build_status(p1, p2, [(p1, p2)])
                emu._send_frame(frame, tag="CHECK-LED")
            else:
                if emu.verbose:
                    import sys
                    print(f"[CHECK][WARN] LED press out of range {p1}-{p2}", file=sys.stderr)
            return

        if cmd == CMD_BTN_RELEASED:
            if emu.verbose:
                import sys
                print(f"[CHECK] LED release from app: {pkt.pin1}-{pkt.pin2}", file=sys.stderr)
            return

        super().handle_packet(pkt, emu)


# --- CONFIG / DIODE_CONFIG / DIODE_CONFIG_DEL ---------------------------------


class ConfigureState(BaseState):
    mode_cmd = CMD_MODE_CONFIGURE
    name = "CONFIGURE"


class DiodeConfigState(BaseState):
    mode_cmd = CMD_MODE_DIODE_CONFIG
    name = "DIODE_CONFIG"


class DiodeConfigDelState(BaseState):
    mode_cmd = CMD_MODE_DIODE_CONFIG_DEL
    name = "DIODE_CONFIG_DEL"
