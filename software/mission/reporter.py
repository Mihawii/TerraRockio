"""Mission report accumulator. Clean-room original. Proprietary.

Collects every spray target found during a run and writes a JSON report: what
was found, where (body-frame mm at time of detection), how confident, and when.
This is the deliverable a grower actually cares about — a map of problems.
"""
from __future__ import annotations
import json
import os
import time
from dataclasses import asdict, dataclass, field

from vision.localizer import SprayTarget


@dataclass
class Finding:
    label: str
    confidence: float
    x_mm: float
    y_mm: float
    z_mm: float
    station: int          # which inspection point along the row
    timestamp: float


@dataclass
class MissionReport:
    started_at: float = field(default_factory=time.time)
    findings: list[Finding] = field(default_factory=list)

    def add(self, target: SprayTarget, station: int) -> None:
        self.findings.append(Finding(
            label=target.label, confidence=target.confidence,
            x_mm=target.x_mm, y_mm=target.y_mm, z_mm=target.z_mm,
            station=station, timestamp=time.time(),
        ))

    def summary(self) -> dict[str, int]:
        counts: dict[str, int] = {}
        for f in self.findings:
            counts[f.label] = counts.get(f.label, 0) + 1
        return counts

    def write(self, directory: str) -> str:
        os.makedirs(directory, exist_ok=True)
        path = os.path.join(directory, f"mission_{int(self.started_at)}.json")
        with open(path, "w") as fh:
            json.dump({
                "started_at": self.started_at,
                "ended_at": time.time(),
                "summary": self.summary(),
                "findings": [asdict(f) for f in self.findings],
            }, fh, indent=2)
        return path
