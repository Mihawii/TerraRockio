# Firmware — ESP32 realtime controller

> **Status: planned / in development.** This directory is scaffolding for the realtime control firmware. No code yet.

The ESP32 is TerraRockio's "spinal cord": a hard-realtime loop that owns the legs and keeps the body balanced, independent of whatever the Pi is doing. See [`../docs/architecture.md`](../docs/architecture.md) for how it fits the system.

## Responsibilities

- **Servo bus master** — drive 18× Dynamixel XL330-M288 over the TTL serial bus; read back position / load / temperature / current.
- **Inverse kinematics** — per-leg 3-DOF IK (coxa / femur / tibia) to turn foot targets into joint angles.
- **Tripod gait** — state machine alternating two tripods; accepts walk/turn/hold goals from the Pi.
- **Balance** — IMU sensor fusion to level the body and detect falls.
- **Safety** — per-servo torque/temperature limits, e-stop, graceful degradation if a leg fails.

## Interfaces

- **Up to Pi Zero 2 W:** serial link receiving goals (`walk`, `turn`, `hold-for-photo`, `step-to <x,y,z>`) and streaming back telemetry.
- **Down to servos:** Dynamixel TTL bus.
- **IMU:** I²C/SPI.

## Likely toolchain

PlatformIO (Arduino or ESP-IDF). `.gitignore` already excludes `.pio/` build artifacts.

## Build order (tracks the [validation plan](../docs/validation-plan.md))

1. Talk to one servo; read its current telemetry → resolves holding-current risk.
2. Single-leg IK + manual foot positioning.
3. Tripod gait on the full robot, flat ground.
4. IMU balance + fall detection.
