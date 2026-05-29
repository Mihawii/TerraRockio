"""Serial link from the Pi (brain) to the ESP32 (spine). Clean-room. Proprietary.

Speaks the exact protocol the firmware expects (see firmware/README.md):
  W vx vy omega | B tx ty tz rx ry rz | U | D | S | H
and parses the TLM reply line. A background thread keeps the heartbeat alive so
the firmware never hits its failsafe, and reads telemetry without blocking the
vision loop.
"""
from __future__ import annotations
import threading
import time
from dataclasses import dataclass

import serial  # pyserial

from config import LinkConfig


@dataclass
class Telemetry:
    voltage_v: float = 0.0
    max_temp_c: float = 0.0
    max_current_ma: float = 0.0
    ik_error: bool = False
    healthy: bool = True
    updated_at: float = 0.0


class ControllerLink:
    def __init__(self, cfg: LinkConfig):
        self._cfg = cfg
        self._ser = serial.Serial(cfg.port, cfg.baud, timeout=0.05)
        self._tlm = Telemetry()
        self._lock = threading.Lock()
        self._stop = threading.Event()
        self._rx = threading.Thread(target=self._reader, daemon=True)
        self._hb = threading.Thread(target=self._heartbeat, daemon=True)

    # -- lifecycle --------------------------------------------------------
    def start(self) -> None:
        self._rx.start()
        self._hb.start()

    def close(self) -> None:
        self._stop.set()
        try:
            self.stop()
            self.sit()
        finally:
            self._ser.close()

    def __enter__(self) -> "ControllerLink":
        self.start()
        return self

    def __exit__(self, *exc) -> None:
        self.close()

    # -- commands ---------------------------------------------------------
    def walk(self, vx: float, vy: float, omega: float) -> None:
        self._send(f"W {vx:.3f} {vy:.3f} {omega:.3f}")

    def set_pose(self, tx=0.0, ty=0.0, tz=0.0, rx=0.0, ry=0.0, rz=0.0) -> None:
        self._send(f"B {tx:.2f} {ty:.2f} {tz:.2f} {rx:.4f} {ry:.4f} {rz:.4f}")

    def stand(self) -> None:
        self._send("U")

    def sit(self) -> None:
        self._send("D")

    def stop(self) -> None:
        self._send("S")

    def telemetry(self) -> Telemetry:
        with self._lock:
            return self._tlm

    # -- internals --------------------------------------------------------
    def _send(self, msg: str) -> None:
        self._ser.write((msg + "\n").encode("ascii"))

    def _heartbeat(self) -> None:
        period = 1.0 / self._cfg.heartbeat_hz
        while not self._stop.is_set():
            self._send("H")
            time.sleep(period)

    def _reader(self) -> None:
        while not self._stop.is_set():
            try:
                raw = self._ser.readline().decode("ascii", "ignore").strip()
            except serial.SerialException:
                break
            if raw.startswith("TLM"):
                self._parse_tlm(raw)

    def _parse_tlm(self, line: str) -> None:
        # "TLM <volt> <maxTemp> <maxCurrent> <ikErr> <healthy>"
        parts = line.split()
        if len(parts) != 6:
            return
        try:
            tlm = Telemetry(
                voltage_v=float(parts[1]),
                max_temp_c=float(parts[2]),
                max_current_ma=float(parts[3]),
                ik_error=parts[4] == "1",
                healthy=parts[5] == "1",
                updated_at=time.time(),
            )
        except ValueError:
            return
        with self._lock:
            self._tlm = tlm
