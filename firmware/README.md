# Firmware — ESP32 realtime controller

The "spinal cord": a fixed-rate loop that owns the legs and balance so the body stays stable no matter what the Pi brain is doing. See [`../docs/architecture.md`](../docs/architecture.md).

> **Status: v0 — written, bench-untested.** The logic is complete and modular; the geometry and per-servo calibration in `include/config.h` are placeholders. Bring up **one leg first**, calibrate, then enable all six. Nothing here has touched real hardware yet.

> **Provenance:** clean-room original, under this repo's proprietary [LICENSE](../LICENSE). The leg solver follows the *standard published* hexapod IK (law-of-cosines; see [Oscar Liang's writeup](https://oscarliang.com/inverse-kinematics-implementation-hexapod-robots/)) — math, not anyone's code. The GPL-licensed ZeroBug project was used only as a behavioural reference and is **not** copied or linked.

## Layout

```
firmware/
├── platformio.ini        ESP32 env + Dynamixel2Arduino dependency
├── include/config.h      ALL tunables: geometry, servo IDs, calibration, limits
└── src/
    ├── kinematics.*      3-DOF leg IK + body-pose transform
    ├── gait.*            tripod gait generator (always ≥3 feet down)
    ├── servo_bus.*       Dynamixel XL330 driver (goal positions + telemetry)
    └── main.cpp          control loop, Pi protocol, safety, IMU hook
```

The split is deliberate: `kinematics` and `gait` are pure math (portable, testable), and only `servo_bus` knows it's talking to XL330s — which is what made moving off ZeroBug's PWM servos a contained change.

## What it does

- **Tripod gait** with smooth (sinusoidal) foot lift; the two tripods stay exactly half a cycle apart, so it can freeze at any phase and stand rock-steady for imaging.
- **Walk + turn** from a single `(vx, vy, omega)` command; **body pose** control (translate/rotate) for aiming the camera or holding still on a slope.
- **XL330 telemetry**: per-servo current / temperature / voltage — the data the [validation plan](../docs/validation-plan.md) needs (holding current!) and the basis for safety + balance.
- **Safety**: heartbeat failsafe (lost Pi → stop and hold), over-temp and low-voltage (3S) cutoffs, and an IK-reachability guard.

## Build

```bash
cd firmware
pio run                 # build
pio run -t upload       # flash an ESP32
pio device monitor      # USB debug @115200
```

## Wiring (default pins — change in code/config)

| Bus | ESP32 | Notes |
|---|---|---|
| Dynamixel TTL | Serial1 RX=16 TX=17, DIR=GPIO5 | half-duplex; needs a direction-control transceiver |
| Pi brain link | Serial2 RX=25 TX=26 | 115200 ASCII protocol |
| USB | Serial | debug logging |

## Pi → controller protocol (newline-terminated)

| Command | Meaning |
|---|---|
| `W <vx> <vy> <omega>` | walk (mm/cycle, mm/cycle, rad/cycle) |
| `B <tx> <ty> <tz> <rx> <ry> <rz>` | body pose (mm, rad) — aim / hold still |
| `U` / `D` | torque on (stand) / off (sit) |
| `S` | stop — freeze on a stable tripod |
| `H` | heartbeat (send faster than `HEARTBEAT_TIMEOUT_MS`) |

Reply line: `TLM <volt> <maxTemp> <maxCurrent_mA> <ikErr> <healthy>`.

## Bring-up order

1. Set each servo's ID to match `SERVO_ID[]` (Dynamixel Wizard).
2. Power **one leg**, flash, and verify coxa/femur/tibia move the right way — flip `SERVO_DIR[]` as needed.
3. Tune `SERVO_OFFSET_DEG[]` so commanded 0 = mechanical neutral.
4. Confirm `FEMUR_LEN`/`TIBIA_LEN`/`COXA_LEN`/`MOUNT_RADIUS` against the final CAD.
5. Log holding current while standing → feed the energy budget.
6. Enable all six; start at low `GAIT_HZ`.

## Next

- Swap the per-servo writes in `servo_bus.cpp` for one **GroupSyncWrite** (smoother at speed).
- Wire a real **IMU** into `imuBalanceTrim()` for self-leveling.
