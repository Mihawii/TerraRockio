# Hardware

Mechanical design for TerraRockio (Design B), authored in Autodesk Fusion 360.

## CAD layout (`cad/`)

| File / folder | What it is |
|---|---|
| `FrameAssembly v35.f3z` | **Master assembly** — the full robot |
| `Fusion/CoxaAssembly v23.f3z` | Coxa (hip) sub-assembly |
| `Fusion/FemurAssembly v13.f3z` | Femur sub-assembly |
| `Fusion/TibiaAndFootAssembly v7.f3z` | Tibia + foot sub-assembly |
| `Fusion/FootAssembly v11.f3z` | Foot sub-assembly |
| `Fusion/LeftLegAssembly v14.f3z` / `RightLegAssembly v14.f3z` | Full left / right legs |
| `Fusion/ServoAssembly v8.f3z` | Servo + horn reference |
| `Fusion/MetalHornCalibrationNeedle.f3d` | Servo-horn calibration aid |
| `Fusion/Parts/` | 26 individual parts (frame panels, leg segments, brackets, wire channels, power compartment, electronics plate, foot tip, switch covers, etc.) |

`.f3z` = Fusion 360 archive (self-contained). `.f3d` = single Fusion design.

## Opening

1. Install [Fusion 360](https://www.autodesk.com/products/fusion-360) (free for personal/startup use).
2. **File → Open** → select a `.f3z`/`.f3d`, or drag it into Fusion.
3. Start with `FrameAssembly v35.f3z` for the whole robot.

## Printing

- **Material: PETG** — chosen for tolerance to greenhouse heat and humidity (PLA would creep/soften).
- Export each part to STL/3MF from Fusion, slice, and print.
- Suggested starting profile (tune to your printer): 0.2 mm layers, 4 walls, 30–40% infill on load-bearing leg parts (coxa/femur/tibia/brackets), lighter on covers and panels.
- **Foot pads:** TPU for grip and shock absorption.

> Print + weigh one leg early — it both validates the printed-frame mass estimate (BOM) and gives you the real stance pose to measure the moment arm ([validation plan](../docs/validation-plan.md) #2).

## Optional: keep STL/STEP exports out of git

Exported meshes are regenerated from the CAD source, so `.gitignore` excludes an `exports/` folder. Create `hardware/cad/exports/` for your slicer-ready files if you want them kept locally but untracked.
