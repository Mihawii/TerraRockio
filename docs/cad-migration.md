# CAD Migration — standard servo → Dynamixel XL330-M288

**Status: open.** The Fusion 360 source in `hardware/cad/` is the *previous* mechanical iteration. It was modeled around a standard-size, 25T-spline servo (the `Servo_SPT5435LV.f3d` part and `25TServoHorn.f3d` give it away). Design B — and every number in `docs/` — uses the much smaller **Dynamixel XL330-M288**. This doc is the spec for bringing the geometry in line.

> ⚠️ This was written from the part filenames and servo datasheets, **not** by opening the binary models. Distances/clearances inside the parts need to be confirmed in Fusion.

## Servo comparison — why this cascades

| | Old (modeled): standard / SPT5435LV class | New (Design B): **Dynamixel XL330-M288** |
|---|---|---|
| Body size (approx) | ~40.5 × 20.2 × 38 mm | **20 × 34 × 22 mm** (datasheet) |
| Mass | ~60 g | **~18–23 g** (datasheet) |
| Output / horn | **25T spline** | M2-screw plastic horn (XL/FR ecosystem) — *not* 25T |
| Mounting | standard servo tabs, 2 screws/side | **M2 screws** into the molded case bosses |
| Interface | PWM | TTL serial bus (daisy-chain) |
| Stall torque | ~35 kgf·cm | ~5.3 kgf·cm |

Because the new servo is smaller, mounts differently, and uses a different horn, every part that touches a servo changes. The frame can likely also shrink (smaller servos → tighter coxa circle), so re-check the ~120 mm coxa circle and 130 × 100 mm chassis afterward.

## Parts to re-model

| Part(s) | Change |
|---|---|
| `Servo_SPT5435LV.f3d` | Replace with the **official Robotis XL330-M288 CAD** (Robotis publishes STEP/3D models — drop it in as the new reference body) |
| `25TServoHorn.f3d` | Remove / replace with the XL330 horn interface (no 25T spline on XL330) |
| `FrameCenterServoHolder.f3d`, `FrameSideServoHolder.f3d` | Resize pockets to the 20 × 34 × 22 mm body; re-pattern to M2 holes |
| `servo-back-hole.f3d` | Re-cut to the XL330 rear/cable feature |
| `CoxaSide1/2.f3d`, `CoxaWireChannelTop.f3d` | Coxa houses a servo — shrink the cavity + remount |
| `FemurSide1/2.f3d`, `FemurWireChannel.f3d` | Femur servo mount — same |
| `TibiaTop.f3d`, `TibiaBottom.f3d` | Tibia servo mount — same |
| `LegConnectorAdapter*.f3d` (Adapter/Top/Middle/Bottom) | Re-fit to the XL330 output interface |
| `FrameV2.f3d`, `ElectronicsPlate.f3d`, `PowerCompartment(Door).f3d`, `ShieldV2.f3d`, `LegRib.f3d`, `tip.f3d`, `micro-switch-holder.f3d`, `SwitchCover.f3d` | Mostly servo-agnostic — only verify bolt-up clearances and whether the frame can shrink |

## Suggested order in Fusion

1. Import the official **XL330-M288** model; make it the canonical servo component.
2. Fix one joint end-to-end (servo holder → mount → horn interface) to lock the new mating pattern.
3. Propagate that pattern to coxa, femur, tibia.
4. Update the leg-connector adapters.
5. Re-check the frame: coxa circle, chassis size, CoM with the lighter legs.
6. Re-export, re-weigh, and confirm against `docs/bill-of-materials.md` (the ~250 g PETG / ~1,006 g total estimates).

## Pointer

Robotis publishes official 3D CAD (STEP) for the XL330 series on their e-Manual / GitHub — use that as the drop-in body rather than re-modeling the servo by hand.
