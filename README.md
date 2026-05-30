# TerraRockio

**A hand-sized autonomous hexapod that walks the rows of a greenhouse, scouting plants and pests — and tells you exactly where to spray.**

TerraRockio is a six-legged robot roughly the size of an adult hand (~290 mm leg span, ~1 kg). It carries a stereo + AI camera (OAK-D Lite) and walks between crop beds to inspect plants leaf-by-leaf: spotting disease, identifying pests, and flagging the precise spots that need treatment — instead of blanket-spraying an entire greenhouse.

> Status: **mechanical design complete in CAD**, full bill of materials and power budget done. Firmware and vision software are in development. See [Status & Confidence](#status--confidence).

---

## Why legs, not wheels or a drone

Greenhouses are a hostile environment for most robots, and that's exactly where a small hexapod wins:

| Challenge | Wheels | Drone | **TerraRockio (legs)** |
|---|---|---|---|
| Gravel, matting, pipes, uneven beds | Gets stuck | N/A | Walks over it |
| 50 mm drip lines & hoses across the floor | Snags | N/A | **Steps over** (75 mm clearance) |
| Tight gaps between plants | Crushes foliage | Prop-wash damages leaves | **Places feet in the gaps** |
| Holding dead-still for a sharp macro shot of a leaf | Rolls/vibrates | Can't hover that precisely | **Freezes mid-stride on a stable tripod** |
| Spreading spores/pests | Low | **High** (prop wash) | Low |
| Losing an actuator mid-mission | Stranded | Crashes | **Limps home** (gait degrades gracefully) |

The key insight: a hexapod always keeps at least three feet planted (a tripod gait), so it can stop and become a rock-steady tripod for imaging — then move on without ever being unstable.

---

## Specs at a glance (Design B)

| | |
|---|---|
| **Mass** | ~1.0 kg (1,006 g modeled) |
| **Legs / joints** | 6 legs × 3 DOF = **18 actuated joints** |
| **Actuators** | 18× Dynamixel XL330-M288 (~23 g, ~5.3 kgf·cm stall) |
| **Leg geometry** | 60 mm femur · 80 mm tibia · ~120 mm coxa mount circle |
| **Chassis** | 130 × 100 × 40 mm |
| **Standing span** | ~290 mm tip-to-tip |
| **Body clearance** | ~75 mm (clears a 50 mm drip line with margin) |
| **Sensing** | OAK-D Lite (stereo depth + Myriad X on-camera AI) + IMU |
| **Compute** | Raspberry Pi Zero 2 W (high-level) + ESP32 (realtime servo/IMU) |
| **Power** | 3S 18650 Li-ion, ~32 Wh, ~141 g |
| **Runtime (est.)** | ~2.6 h ideal → **~1.8–2 h** real-world |
| **Frame** | PETG, 3D-printed |

Full reasoning behind every number: [`docs/design-spec.md`](docs/design-spec.md).

---

## System architecture

```
                    ┌─────────────────────────────────────┐
                    │            TerraRockio                │
                    │                                       │
   plants/pests ───►│  OAK-D Lite ──► Pi Zero 2 W           │
   (vision)         │  (stereo+AI)    • perception          │──► spray targets,
                    │                 • mapping/nav         │    plant-health reports
                    │                 • gait planning       │
                    │                      │                │
                    │                      ▼ (serial)       │
                    │                  ESP32                │
                    │                 • realtime gait loop  │
                    │                 • IMU fusion / balance│
                    │                      │                │
                    │                      ▼ (TTL bus)      │
                    │            18× Dynamixel XL330         │
                    │             (6 legs × 3 joints)       │
                    │                                       │
                    │   3S 18650 ──► buck ──► 5V rail        │
                    └─────────────────────────────────────┘
```

- **Pi Zero 2 W** — the "brain": runs the OAK-D vision pipeline, builds a picture of the row, decides where to go and where to spray.
- **ESP32** — the "spinal cord": the hard-realtime loop that runs the gait and reads the IMU so the body stays balanced, independent of whatever the Pi is busy doing.
- **OAK-D Lite** — does plant/pest inference on its own Myriad X chip, so the Pi isn't bottlenecked.

Details: [`docs/architecture.md`](docs/architecture.md).

---

## Repository layout

```
TerraRockio/
├── README.md                  ← you are here
├── docs/
│   ├── design-spec.md          Geometry, kinematics, torque & servo analysis (Design B)
│   ├── bill-of-materials.md    Full BOM with masses and sourcing
│   ├── energy-budget.md        Battery, duty cycle, runtime math + derating
│   ├── architecture.md         Compute / sensing / actuation / power architecture
│   ├── validation-plan.md      What still needs bench testing, and how to test it
│   ├── cad-migration.md        Bringing the CAD up to the XL330 servo (in progress)
│   └── leg-design.md           Beginner build: a leg from 3 servos + 3 printed plates
├── hardware/
│   ├── README.md               CAD overview + print settings
│   └── cad/                     Fusion 360 source (master assembly + legs + 26 parts)
├── firmware/                   ESP32 realtime controller (planned)
└── software/                   Pi Zero 2 W perception + navigation (planned)
```

---

## Status & confidence

Being honest about what's measured vs. estimated, because it changes what you'd trust:

| Confidence | What |
|---|---|
| ✅ **Datasheet-grade** | Servo mass & stall torque, battery cell energy, camera mass |
| 🟡 **Engineering estimate (±20%)** | Frame mass, average power draw, runtime |
| 🔬 **Needs bench validation** | Servo *holding* current, real horizontal moment arm in stance, peak transient torque under gait, and the 90%-stationary duty-cycle assumption |

That last bucket is the difference between a spec sheet and a working robot — the plan to close it is in [`docs/validation-plan.md`](docs/validation-plan.md). The single biggest open question is the duty cycle, which one conversation with a greenhouse grower would settle.

---

## Roadmap

- [x] **Phase 0 — Mechanical design.** Full hexapod in Fusion 360: frame, 3-DOF legs, servo brackets, power compartment, electronics plate. ✔
- [x] **Phase 0 — Systems budget.** BOM, mass budget, torque analysis, energy/runtime model. ✔
- [ ] **Phase 1 — Print & assemble** one leg, validate torque and holding current on the bench.
- [ ] **Phase 2 — ESP32 firmware:** Dynamixel bus, single-leg IK, tripod gait, IMU balance.
- [ ] **Phase 3 — Full build** (18 servos) + walking on flat ground.
- [ ] **Phase 4 — OAK-D vision:** plant detection, pest/disease classification, spray-target localization.
- [ ] **Phase 5 — Autonomy:** row navigation, mapping, mission reports.
- [ ] **Phase 6 — Greenhouse field trial.**

---

## Working with the CAD

The design lives in [`hardware/cad/`](hardware/cad/) as Fusion 360 archives (`.f3z`) and parts (`.f3d`).

1. Install [Autodesk Fusion 360](https://www.autodesk.com/products/fusion-360) (free for personal/startup use).
2. Open `hardware/cad/FrameAssembly v35.f3z` for the full robot, or any sub-assembly / part individually.
3. Print parts in **PETG** (chosen for greenhouse heat + humidity tolerance). Per-part settings: [`hardware/README.md`](hardware/README.md).

---

## License

See [LICENSE](LICENSE).
