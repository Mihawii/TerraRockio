"""TerraRockio Pi-side configuration. Clean-room original. Proprietary (see /LICENSE).

Every tunable for the brain lives here: camera, model, the serial link to the
ESP32 controller, and the camera-on-body mounting pose. Mirrors the firmware's
`config.h` philosophy.
"""
from __future__ import annotations
from dataclasses import dataclass, field


@dataclass(frozen=True)
class CameraConfig:
    preview_size: tuple[int, int] = (300, 300)   # NN input
    confidence_threshold: float = 0.5
    # Depth gating (mm): ignore detections nearer/farther than a useful leaf range.
    depth_min_mm: int = 100
    depth_max_mm: int = 2000
    bbox_scale: float = 0.5   # shrink bbox before averaging depth (less edge noise)


@dataclass(frozen=True)
class ModelConfig:
    # Compiled .blob for the Myriad X. Not committed — see software/README.md
    # ("the dataset question") for how this gets trained/converted.
    blob_path: str = "models/plant_pest.blob"
    # Detector classes. 'spray' classes are what triggers a treatment target.
    labels: tuple[str, ...] = ("background", "healthy_leaf", "aphid",
                               "whitefly", "powdery_mildew", "leaf_spot")
    spray_labels: tuple[str, ...] = ("aphid", "whitefly", "powdery_mildew", "leaf_spot")


@dataclass(frozen=True)
class LinkConfig:
    port: str = "/dev/serial0"   # Pi UART to ESP32
    baud: int = 115200
    heartbeat_hz: float = 5.0    # must beat firmware HEARTBEAT_TIMEOUT_MS


@dataclass(frozen=True)
class MountConfig:
    """Camera pose on the body. DepthAI spatial coords are mm: x=right, y=up,
    z=forward(depth) from the camera. Body frame: x=forward, y=left, z=up.
    Calibrate these signs/offsets once the camera is mounted (TUNE)."""
    offset_forward_mm: float = 60.0   # camera sits ahead of body centre
    offset_up_mm: float = 40.0
    pitch_down_deg: float = 20.0      # tilted down toward the plants


@dataclass(frozen=True)
class MissionConfig:
    step_forward_mm: float = 40.0     # how far to advance between inspections
    inspect_seconds: float = 1.5      # dwell time per inspection point
    walk_vx_per_cycle: float = 12.0   # mm/cycle sent as walk speed
    report_path: str = "reports"


@dataclass(frozen=True)
class Config:
    camera: CameraConfig = field(default_factory=CameraConfig)
    model: ModelConfig = field(default_factory=ModelConfig)
    link: LinkConfig = field(default_factory=LinkConfig)
    mount: MountConfig = field(default_factory=MountConfig)
    mission: MissionConfig = field(default_factory=MissionConfig)


CONFIG = Config()
