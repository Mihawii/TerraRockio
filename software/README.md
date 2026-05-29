# Software — Pi Zero 2 W perception & autonomy

> **Status: planned / in development.** This directory is scaffolding for the high-level brain. No code yet.

The Raspberry Pi Zero 2 W is TerraRockio's "brain": it runs the vision pipeline, decides where to go and where to spray, and reports back. It sends *goals* to the ESP32 firmware, which handles the realtime walking. See [`../docs/architecture.md`](../docs/architecture.md).

## Responsibilities

- **Vision pipeline** — drive the OAK-D Lite via [DepthAI](https://docs.luxonis.com/). Run plant-health, pest, and disease inference *on the camera's Myriad X VPU* so the Pi's CPU stays free.
- **Spray-target localization** — fuse detections with stereo depth to give each target real 3D coordinates, then hand a `step-to`/`treat-here` goal to the firmware.
- **Row navigation & mapping** — move down crop rows, track where it's been, avoid obstacles.
- **Reporting** — per-mission plant-health and pest maps over Wi-Fi.

## Interfaces

- **OAK-D Lite:** USB (DepthAI / OpenCV).
- **Down to ESP32:** serial — sends goals, receives gait/balance telemetry.
- **Out:** Wi-Fi telemetry / mission reports.

## Likely stack

Python 3 + DepthAI + OpenCV. `.gitignore` already excludes `.venv/` and Python caches.

## Build order (after the robot walks — firmware phases 1–3)

1. OAK-D streaming + depth on the Pi.
2. Plant detection; then pest/disease classification.
3. Project detections to 3D spray targets.
4. Row navigation + mission reports.

## The dataset question

Pest/disease classifiers need labeled greenhouse imagery. Early field visits should double as data-collection runs (drive, image, label) — and the same grower conversation that settles the [duty cycle](../docs/validation-plan.md) is the moment to learn which pests/diseases actually matter to them.
