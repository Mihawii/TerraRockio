"""TerraRockio brain entrypoint. Clean-room original. Proprietary (see /LICENSE).

Wires the camera, the controller link, and the scout mission together, runs one
scouting pass down a row, and writes the report. Ctrl-C sits the robot safely.

    python3 main.py [--stations N] [--dry-run]
"""
from __future__ import annotations
import argparse
import sys

from config import CONFIG
from controller_link import ControllerLink
from mission.scout import Scout
from vision.localizer import Localizer
from vision.pipeline import Camera


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description="TerraRockio row scouting pass")
    ap.add_argument("--stations", type=int, default=20,
                    help="inspection points before the pass ends")
    ap.add_argument("--dry-run", action="store_true",
                    help="run vision only; never command the legs")
    args = ap.parse_args(argv)

    localizer = Localizer(CONFIG.mount, CONFIG.model)

    try:
        with Camera(CONFIG.camera, CONFIG.model) as camera, \
                ControllerLink(CONFIG.link) as link:
            if args.dry_run:
                _watch(camera, localizer)
                return 0
            scout = Scout(CONFIG, link, camera, localizer, args.stations)
            report = scout.run()
            path = report.write(CONFIG.mission.report_path)
            print(f"\nmission summary: {report.summary()}\nreport: {path}")
    except KeyboardInterrupt:
        print("\ninterrupted — sitting down")
    return 0


def _watch(camera: Camera, localizer: Localizer) -> None:
    """Vision-only loop: print spray targets without moving. Good for bring-up."""
    print("dry-run: vision only, legs idle. Ctrl-C to stop.")
    while True:
        for t in localizer.targets(camera.poll()):
            print(f"{t.label:14s} conf={t.confidence:.2f}  "
                  f"fwd={t.x_mm:6.0f} left={t.y_mm:6.0f} up={t.z_mm:6.0f} mm")


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
