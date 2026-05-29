# Bill of Materials — Design B

Mass budget totaling **~1,006 g**. This is the number the whole design (torque, runtime, stability) is built around.

| Item | Qty | Mass | Notes |
|---|---:|---:|---|
| Dynamixel XL330-M288 servo | 18 | **414 g** | 6 legs × 3 DOF; ~23 g each |
| PETG frame + leg segments + brackets | — | **~250 g** | 3D-printed; see `hardware/cad/` |
| 3S Li-ion battery (3× 18650) | 1 | **~141 g** | ~32 Wh; keep low & central |
| OAK-D Lite camera | 1 | **~61 g** | stereo depth + Myriad X AI |
| Pi Zero 2 W + ESP32 + IMU + buck + wiring | 1 set | **~90 g** | compute + power conversion |
| Fasteners, foot pads, misc | — | **~50 g** | M2/M2.5 hardware, TPU foot pads |
| **Total** | | **~1,006 g** | |

## Confidence per line

| Confidence | Items |
|---|---|
| ✅ Datasheet-grade | Servo mass, battery cell energy, camera mass |
| 🟡 Estimate (±20%) | PETG frame mass, wiring/misc |

Frame mass is the main soft number — it'll be exact once the parts are sliced and weighed (the slicer reports it per part, and a kitchen scale confirms it after printing).

## Key components — sourcing notes

- **Dynamixel XL330-M288** — Robotis. Smart TTL-bus servos; daisy-chain wiring, per-servo position/load/temp/current telemetry.
- **OAK-D Lite** — Luxonis. On-camera Myriad X VPU runs vision inference, offloading the Pi.
- **Raspberry Pi Zero 2 W** — quad-core, Wi-Fi; high-level brain.
- **ESP32** — realtime servo + IMU loop.
- **3× 18650** in 3S — standard cells; choose a reputable brand and a 3S BMS.
- **Buck converter** — steps the 3S pack (10.8–12.6 V) down to the 5 V logic/servo rail.
