# Software — Pi Zero 2 W brain (perception + autonomy)

The "brain": runs the OAK-D vision, decides where to inspect and what to flag, and sends *goals* to the ESP32 firmware (which owns the realtime walking). See [`../docs/architecture.md`](../docs/architecture.md).

> **Status: v0 — written, untested on hardware.** Logic is complete; it needs a real OAK-D, a flashed controller, and a trained model blob. Clean-room original under this repo's proprietary [LICENSE](../LICENSE) — no third-party code copied.

## Layout

```
software/
├── main.py            entrypoint: one scouting pass, writes a report
├── config.py          all tunables: camera, model, serial link, camera mount pose
├── controller_link.py serial protocol to the ESP32 (+ heartbeat & telemetry threads)
├── vision/
│   ├── pipeline.py    OAK-D DepthAI pipeline + decoded detections
│   └── localizer.py   camera-frame detections → body-frame spray targets
└── mission/
    ├── scout.py       walk → stop → inspect → repeat state machine
    └── reporter.py    accumulates findings → JSON mission report
```

## How it works

1. **`vision/pipeline.py`** builds a DepthAI pipeline where a *spatial* detection network runs the plant/pest model on the camera's Myriad X **and fuses each hit with stereo depth** — so detections arrive with a real 3D position (mm). The Pi's modest CPU never runs the neural net.
2. **`vision/localizer.py`** rotates/translates those camera-frame points into the **body frame** (x=forward, y=left, z=up) — the same frame the firmware's pose/step commands use — and keeps only the classes worth spraying.
3. **`mission/scout.py`** runs the loop: advance a step, freeze on a stable tripod, inspect for a dwell window, log targets, repeat. It checks controller health each cycle and stops if the robot is unhappy.
4. **`mission/reporter.py`** writes a JSON map of every problem found: label, 3D location, confidence, timestamp.

## Run

```bash
pip install -r requirements.txt
python3 main.py --dry-run      # vision only, legs idle — best for first bring-up
python3 main.py --stations 20  # full scouting pass
```

`--dry-run` prints detected spray targets with their 3D positions and never commands the legs — the safe way to validate the camera before the robot can walk.

## The model blob (the real work ahead)

`models/plant_pest.blob` is **not** in the repo — it's the trained pest/disease detector compiled for the Myriad X, and it needs labeled greenhouse imagery to exist. The plan:

1. Collect imagery on early field visits (the same grower visit that settles the [duty-cycle question](../docs/validation-plan.md)).
2. Label the pests/diseases that grower actually cares about.
3. Train a MobileNet-SSD-class detector, convert to OpenVINO IR, compile to `.blob`.

Until then, run real hardware with any public MobileNet blob to exercise the pipeline; swap in the trained model when ready. The `labels`/`spray_labels` lists in `config.py` define the detector's classes.

## Next

- Real **row navigation** (currently dead-reckoned steps) — odometry/visual tracking slots into `scout.py`.
- Stream annotated frames to a web view for live monitoring.
- Close the loop: send a `step-to`/treat command to the firmware for each spray target.
