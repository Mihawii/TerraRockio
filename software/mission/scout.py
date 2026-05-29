"""Row-scouting state machine. Clean-room original. Proprietary.

Drives the high-level behaviour: advance a step, stop and hold a steady tripod,
inspect (run detections, log spray targets), repeat. Deliberately simple and
readable — real row navigation / SLAM is future work; this is the loop those
slot into. The firmware owns *how* to walk; this owns *when* and *why*.
"""
from __future__ import annotations
import enum
import time

from config import Config
from controller_link import ControllerLink
from vision.localizer import Localizer
from vision.pipeline import Camera
from mission.reporter import MissionReport


class State(enum.Enum):
    STAND = enum.auto()
    WALK = enum.auto()
    INSPECT = enum.auto()
    DONE = enum.auto()


class Scout:
    def __init__(self, cfg: Config, link: ControllerLink, camera: Camera,
                 localizer: Localizer, max_stations: int = 20):
        self._cfg = cfg
        self._link = link
        self._camera = camera
        self._localizer = localizer
        self._max_stations = max_stations
        self._state = State.STAND
        self._station = 0
        self.report = MissionReport()

    def run(self) -> MissionReport:
        self._link.stand()
        time.sleep(1.0)
        self._state = State.WALK
        while self._state is not State.DONE:
            if not self._safe():
                break
            getattr(self, f"_do_{self._state.name.lower()}")()
        self._link.stop()
        return self.report

    # -- per-state behaviour ---------------------------------------------
    def _do_walk(self) -> None:
        # Advance one step, then stop for a stable inspection.
        m = self._cfg.mission
        self._link.walk(m.walk_vx_per_cycle, 0.0, 0.0)
        time.sleep(m.step_forward_mm / max(m.walk_vx_per_cycle, 1e-3) * 0.1)
        self._link.walk(0.0, 0.0, 0.0)   # freeze on a tripod
        self._state = State.INSPECT

    def _do_inspect(self) -> None:
        # Hold still and watch for the dwell window; log any spray targets.
        deadline = time.time() + self._cfg.mission.inspect_seconds
        seen = 0
        while time.time() < deadline:
            for target in self._localizer.targets(self._camera.poll()):
                self.report.add(target, self._station)
                seen += 1
            time.sleep(0.05)
        print(f"station {self._station}: {seen} target(s)")
        self._station += 1
        self._state = State.DONE if self._station >= self._max_stations else State.WALK

    def _do_stand(self) -> None:
        self._link.stand()
        self._state = State.WALK

    # -- safety -----------------------------------------------------------
    def _safe(self) -> bool:
        t = self._link.telemetry()
        if not t.healthy:
            print(f"controller unhealthy (V={t.voltage_v} T={t.max_temp_c}) — stopping")
            return False
        return True
