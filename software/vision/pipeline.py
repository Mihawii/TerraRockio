"""OAK-D Lite pipeline + detection stream. Clean-room original. Proprietary.

Builds a DepthAI pipeline that runs the plant/pest detector on the camera's
Myriad X VPU AND fuses each detection with the stereo depth map, so every
detection comes back with a real 3D position (mm) — no extra work on the Pi.
That spatial output is exactly what turns "there's an aphid" into "spray here".
"""
from __future__ import annotations
from dataclasses import dataclass

import depthai as dai

from config import CameraConfig, ModelConfig


@dataclass
class Detection:
    label: str
    confidence: float
    # Spatial position in the CAMERA frame (mm): x=right, y=up, z=forward.
    x_mm: float
    y_mm: float
    z_mm: float
    # Normalized bbox (0..1) in the preview frame.
    bbox: tuple[float, float, float, float]


def build_pipeline(cam_cfg: CameraConfig, model_cfg: ModelConfig) -> dai.Pipeline:
    pipeline = dai.Pipeline()

    # --- RGB (detector input) ---
    cam = pipeline.create(dai.node.ColorCamera)
    cam.setPreviewSize(*cam_cfg.preview_size)
    cam.setResolution(dai.ColorCameraProperties.SensorResolution.THE_1080_P)
    cam.setBoardSocket(dai.CameraBoardSocket.CAM_A)
    cam.setInterleaved(False)

    # --- Stereo depth ---
    mono_l = pipeline.create(dai.node.MonoCamera)
    mono_r = pipeline.create(dai.node.MonoCamera)
    mono_l.setBoardSocket(dai.CameraBoardSocket.CAM_B)
    mono_r.setBoardSocket(dai.CameraBoardSocket.CAM_C)
    for m in (mono_l, mono_r):
        m.setResolution(dai.MonoCameraProperties.SensorResolution.THE_400_P)

    stereo = pipeline.create(dai.node.StereoDepth)
    stereo.setDefaultProfilePreset(dai.node.StereoDepth.PresetMode.HIGH_DENSITY)
    stereo.setDepthAlign(dai.CameraBoardSocket.CAM_A)
    mono_l.out.link(stereo.left)
    mono_r.out.link(stereo.right)

    # --- Spatial detection NN (detection + depth fusion on-device) ---
    nn = pipeline.create(dai.node.MobileNetSpatialDetectionNetwork)
    nn.setBlobPath(model_cfg.blob_path)
    nn.setConfidenceThreshold(cam_cfg.confidence_threshold)
    nn.setBoundingBoxScaleFactor(cam_cfg.bbox_scale)
    nn.setDepthLowerThreshold(cam_cfg.depth_min_mm)
    nn.setDepthUpperThreshold(cam_cfg.depth_max_mm)
    cam.preview.link(nn.input)
    stereo.depth.link(nn.inputDepth)

    out = pipeline.create(dai.node.XLinkOut)
    out.setStreamName("detections")
    nn.out.link(out.input)
    return pipeline


class Camera:
    """Owns the device and yields decoded detections."""

    def __init__(self, cam_cfg: CameraConfig, model_cfg: ModelConfig):
        self._labels = model_cfg.labels
        self._pipeline = build_pipeline(cam_cfg, model_cfg)
        self._device: dai.Device | None = None
        self._queue = None

    def __enter__(self) -> "Camera":
        self._device = dai.Device(self._pipeline)
        self._queue = self._device.getOutputQueue("detections", maxSize=4, blocking=False)
        return self

    def __exit__(self, *exc) -> None:
        if self._device is not None:
            self._device.close()

    def poll(self) -> list[Detection]:
        """Latest detections, or [] if no new frame is ready."""
        msg = self._queue.tryGet()
        if msg is None:
            return []
        results: list[Detection] = []
        for d in msg.detections:
            label = self._labels[d.label] if d.label < len(self._labels) else str(d.label)
            results.append(Detection(
                label=label,
                confidence=float(d.confidence),
                x_mm=float(d.spatialCoordinates.x),
                y_mm=float(d.spatialCoordinates.y),
                z_mm=float(d.spatialCoordinates.z),
                bbox=(float(d.xmin), float(d.ymin), float(d.xmax), float(d.ymax)),
            ))
        return results
