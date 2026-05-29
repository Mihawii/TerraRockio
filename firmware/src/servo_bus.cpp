// TerraRockio — Dynamixel XL330 bus driver. Clean-room. Proprietary.
#include "servo_bus.h"
#include <Dynamixel2Arduino.h>

namespace {
using config::NUM_SERVOS;

// Bus on Serial1. On ESP32, set the UART pins to your wiring below.
constexpr int DXL_RX_PIN = 16;
constexpr int DXL_TX_PIN = 17;
Dynamixel2Arduino dxl(Serial1, config::DXL_DIR_PIN);

ServoTelemetry g_tlm;
bool g_healthy = true;

// Joint angle (rad) -> XL330 raw tick, applying per-servo direction + offset.
uint32_t angleToTicks(int idx, float rad) {
  float deg = rad * config::RAD2DEG;
  deg = config::SERVO_DIR[idx] * (deg + config::SERVO_OFFSET_DEG[idx]);
  long ticks = lroundf(config::TICK_CENTER + deg * config::TICKS_PER_DEG);
  if (ticks < 0)    ticks = 0;
  if (ticks > 4095) ticks = 4095;
  return (uint32_t)ticks;
}
} // namespace

bool servobus::begin() {
  Serial1.begin(config::DXL_BAUD, SERIAL_8N1, DXL_RX_PIN, DXL_TX_PIN);
  dxl.begin(config::DXL_BAUD);
  dxl.setPortProtocolVersion(config::DXL_PROTOCOL);

  bool allFound = true;
  for (int i = 0; i < NUM_SERVOS; ++i) {
    const uint8_t id = config::SERVO_ID[i];
    if (!dxl.ping(id)) { allFound = false; continue; }   // missing servo
    dxl.torqueOff(id);
    dxl.setOperatingMode(id, OP_POSITION);               // 0..360 position control
    dxl.torqueOn(id);
  }
  g_healthy = allFound;
  return allFound;
}

void servobus::writeLegAngles(const LegAngles legs[config::NUM_LEGS]) {
  // TODO(perf): replace the per-servo loop with a single GroupSyncWrite of all
  // 18 goal positions to remove cross-joint phase skew at high gait speeds.
  for (int leg = 0; leg < config::NUM_LEGS; ++leg) {
    const float j[3] = { legs[leg].coxa, legs[leg].femur, legs[leg].tibia };
    for (int joint = 0; joint < config::DOF; ++joint) {
      const int idx = leg * config::DOF + joint;
      dxl.setGoalPosition(config::SERVO_ID[idx], angleToTicks(idx, j[joint]));
    }
  }
}

void servobus::torque(bool on) {
  for (int i = 0; i < NUM_SERVOS; ++i) {
    if (on) dxl.torqueOn(config::SERVO_ID[i]);
    else    dxl.torqueOff(config::SERVO_ID[i]);
  }
}

void servobus::pollTelemetry(ServoTelemetry& t) {
  // Reading all 18 every cycle is too slow; sample one per call (round-robin).
  static int i = 0;
  const uint8_t id = config::SERVO_ID[i];
  g_tlm.current_mA[i] = dxl.getPresentCurrent(id, UNIT_MILLI_AMPERE);
  g_tlm.temp_C[i]     = dxl.readControlTableItem(PRESENT_TEMPERATURE, id);
  g_tlm.voltage_V     = dxl.readControlTableItem(PRESENT_INPUT_VOLTAGE, id) * 0.1f;

  if (g_tlm.temp_C[i] > config::MAX_TEMP_C ||
      (g_tlm.voltage_V > 1.0f && g_tlm.voltage_V < config::LOW_VOLT_CUTOFF_V)) {
    g_healthy = false;
  }
  i = (i + 1) % NUM_SERVOS;
  t = g_tlm;
}

bool servobus::healthy() { return g_healthy; }
