# System Architecture

TerraRockio splits work across two processors by timescale: a **realtime controller** that must never miss a beat (the gait), and a **high-level brain** that can afford to think (perception, navigation). This is the standard "spinal cord + brain" split, and it's why the robot stays balanced even when vision processing is slow.

```
   ┌──────────────────────────────────────────────────────────────┐
   │ PERCEPTION & PLANNING                  Raspberry Pi Zero 2 W   │
   │                                                                │
   │   OAK-D Lite ──USB──►  • DepthAI vision pipeline               │
   │   (stereo + Myriad X)  • plant health / pest / disease ID      │
   │                        • spray-target localization (3D)        │
   │                        • row mapping & navigation              │
   │                        • mission reports / telemetry (Wi-Fi)   │
   └───────────────────────────────┬────────────────────────────────┘
                                    │  serial (gait goals, mode)
                                    ▼
   ┌──────────────────────────────────────────────────────────────┐
   │ REALTIME CONTROL                                      ESP32     │
   │   • tripod gait state machine + leg inverse kinematics         │
   │   • IMU sensor fusion → body leveling / balance                │
   │   • servo bus master (position/load/temp telemetry)            │
   │   • safety: torque/temp limits, e-stop, fall detection         │
   └───────────────────────────────┬────────────────────────────────┘
                                    │  TTL serial bus (daisy-chain)
                                    ▼
   ┌──────────────────────────────────────────────────────────────┐
   │ ACTUATION        18× Dynamixel XL330-M288                       │
   │                  6 legs × (coxa, femur, tibia)                  │
   └──────────────────────────────────────────────────────────────┘

   POWER:  3S 18650 (~32 Wh) ──► BMS ──► buck ──► 5 V rail ──► servos + logic
                                                  └► IMU, Pi, ESP32, OAK-D
```

## Why two processors

| Concern | Lives on | Reason |
|---|---|---|
| Gait loop, balance | **ESP32** | Must run at a fixed, jitter-free rate; a stutter here is a stumble. Keep it off the Linux scheduler. |
| Vision, mapping, decisions | **Pi Zero 2 W** | Heavy, bursty, and tolerant of latency — fine on Linux. |
| Plant/pest inference | **OAK-D Lite (on-camera)** | The Myriad X VPU runs the neural nets, so the Pi isn't the bottleneck and the Pi Zero's modest CPU stays free. |

If the Pi is mid-inference and slow, the ESP32 keeps the robot standing and balanced regardless. The Pi sends *goals* ("walk forward", "hold for a photo", "step to this target"); the ESP32 owns *how*.

## Sensing

- **OAK-D Lite** — stereo depth + RGB + on-camera AI. Gives both *what* (is this a pest / is this leaf diseased) and *where in 3D* (so a spray target has real coordinates).
- **IMU** — feeds the ESP32 balance loop and detects falls.
- **Per-servo telemetry** — position, load, temperature, current over the Dynamixel bus; used for balance, safety limits, and live health monitoring.

## Power tree

3S 18650 pack → BMS → buck converter → 5 V rail feeding servos and all logic. Battery is mounted **low and central** to keep the center of mass inside the stance support triangle (see [design-spec.md](design-spec.md) §6).
