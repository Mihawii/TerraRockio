"""Turn camera-frame detections into body-frame spray targets. Clean-room. Proprietary.

DepthAI gives positions in the camera frame (x=right, y=up, z=forward, mm).
This rotates by the camera's mounting tilt and translates by its offset to get
positions in the robot's body frame (x=forward, y=left, z=up) — the frame the
firmware's body-pose / step-to commands use.
"""
from __future__ import annotations
import math
from dataclasses import dataclass

from config import ModelConfig, MountConfig
from vision.pipeline import Detection


@dataclass
class SprayTarget:
    label: str
    confidence: float
    # Body frame (mm): x=forward, y=left, z=up.
    x_mm: float
    y_mm: float
    z_mm: float


class Localizer:
    def __init__(self, mount: MountConfig, model: ModelConfig):
        self._mount = mount
        self._spray_labels = set(model.spray_labels)
        self._pitch = math.radians(mount.pitch_down_deg)

    def _camera_to_body(self, x_r: float, y_u: float, z_f: float) -> tuple[float, float, float]:
        # Camera axes -> body axes (before tilt): forward=z, left=-x, up=y.
        fwd, left, up = z_f, -x_r, y_u
        # Undo the downward pitch about the body's left(y) axis.
        c, s = math.cos(self._pitch), math.sin(self._pitch)
        fwd_p = fwd * c - up * s
        up_p = fwd * s + up * c
        # Translate by the mount offset.
        return (fwd_p + self._mount.offset_forward_mm,
                left,
                up_p + self._mount.offset_up_mm)

    def targets(self, detections: list[Detection]) -> list[SprayTarget]:
        out: list[SprayTarget] = []
        for d in detections:
            if d.label not in self._spray_labels:
                continue
            if d.z_mm <= 0:           # invalid/empty depth
                continue
            x, y, z = self._camera_to_body(d.x_mm, d.y_mm, d.z_mm)
            out.append(SprayTarget(d.label, d.confidence, x, y, z))
        return out
