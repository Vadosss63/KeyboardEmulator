from __future__ import annotations

from typing import Optional

try:
    import serial  # pyserial
except ImportError:
    serial = None


class SerialPort:
 
    def __init__(
        self,
        port: str,
        baudrate: int,
        *,
        timeout: float = 0.0,
        write_timeout: float = 1.0,
        verbose: bool = False,
    ) -> None:
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.write_timeout = write_timeout
        self.verbose = verbose

        self._ser: Optional["serial.Serial"] = None

    def open(self) -> None:
        if serial is None:
            raise RuntimeError("pyserial is not installed. Install it with: pip install pyserial")

        if self._ser is not None and self._ser.is_open:
            return

        self._ser = serial.Serial(
            self.port,
            self.baudrate,
            timeout=self.timeout,
            write_timeout=self.write_timeout,
        )
        if self.verbose:
            import sys
            print(f"[SerialPort] Opened {self.port} @ {self.baudrate}", file=sys.stderr)

    def close(self) -> None:
        if self._ser is not None:
            try:
                self._ser.close()
            finally:
                self._ser = None

    @property
    def is_open(self) -> bool:
        return self._ser is not None and self._ser.is_open

    def read(self, size: int = 1) -> bytes:
        if not self.is_open:
            return b""
        return self._ser.read(size)

    def write(self, data: bytes) -> int:
        if not self.is_open:
            raise RuntimeError("SerialPort.write() called while port is closed")
        return self._ser.write(data)

    def __enter__(self) -> "SerialPort":
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb) -> None:
        self.close()
