from __future__ import annotations

from dataclasses import dataclass
from typing import Callable, Iterable, List, Tuple

PROTOCOL_SOF = 0xAA

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
CMD_STATUS_UPDATE         = 0x0C


def calc_checksum(data: bytes) -> int:
    return sum(data) & 0xFF


@dataclass
class Packet:
    command: int
    payload: bytes

    @property
    def pin1(self) -> int:
        return self.payload[0] if len(self.payload) >= 1 else 0

    @property
    def pin2(self) -> int:
        return self.payload[1] if len(self.payload) >= 2 else 0


class Framer:
    def __init__(self, on_packet: Callable[[Packet], None], verbose: bool = False):
        self.on_packet = on_packet
        self.verbose = verbose
        self.buf = bytearray()

    def feed(self, chunk: bytes) -> None:
        self.buf.extend(chunk)
        self._drain()

    def _drain(self) -> None:
        import sys

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
            expected_total = 1 + 1 + length
            if len(self.buf) < expected_total:
                return

            frame = bytes(self.buf[:expected_total])
            del self.buf[:expected_total]

            checksum_expected = frame[-1]
            checksum_calc = calc_checksum(frame[:-1])
            if checksum_expected != checksum_calc:
                if self.verbose:
                    print(
                        f"[WARN] Bad checksum: got=0x{checksum_expected:02X}, "
                        f"calc=0x{checksum_calc:02X}",
                        file=sys.stderr,
                    )
                continue

            cmd = frame[2]
            payload = frame[3:-1]  # exclude SOF, Length, Command, Checksum

            self.on_packet(Packet(cmd, payload))


def build_packet(command: int, payload: bytes | Iterable[int] = b"") -> bytes:
    
    if not isinstance(payload, (bytes, bytearray)):
        payload = bytes(int(b) & 0xFF for b in payload)

    # [SOF, Length, Command, Payload..., Checksum]
    # Length = 1 (Command) + len(Payload) + 1 (Checksum)
    length = 1 + len(payload) + 1

    header_and_body_wo_crc = bytes([PROTOCOL_SOF, length, command & 0xFF]) + payload
    crc = calc_checksum(header_and_body_wo_crc)
    return header_and_body_wo_crc + bytes([crc & 0xFF])


def build_cmd_no_payload(command: int) -> bytes:
    return build_packet(command, b"")


def build_cmd_with_pins(command: int, pin1: int, pin2: int) -> bytes:
    return build_packet(command, [pin1, pin2])


def build_status(pin1: int, pin2: int, led_pairs: List[Tuple[int, int]]) -> bytes:
    leds_num = len(led_pairs) & 0xFF
    payload = bytearray([pin1 & 0xFF, pin2 & 0xFF, leds_num])

    for (a, k) in led_pairs:
        payload.append(a & 0xFF)
        payload.append(k & 0xFF)

    return build_packet( CMD_STATUS_UPDATE, bytes(payload) )
