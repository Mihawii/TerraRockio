// TerraRockio — Dynamixel XL330 bus driver. Clean-room original. Proprietary.
//
// Wraps the 18 servos behind a small interface: push joint angles, read back
// health telemetry (the per-servo current/temperature that PWM servos can't
// give us, and that the energy/torque validation plan needs).
#pragma once
#include "kinematics.h"

struct ServoTelemetry {
  float current_mA[config::NUM_SERVOS] = {0};
  float temp_C[config::NUM_SERVOS]     = {0};
  float voltage_V = 0;
};

namespace servobus {

bool begin();                                       // open bus, configure all 18
void writeLegAngles(const LegAngles legs[config::NUM_LEGS]);  // -> goal positions
void torque(bool on);
void pollTelemetry(ServoTelemetry& t);              // round-robin: one servo/call
bool healthy();                                     // false on over-temp / brown-out

} // namespace servobus
