// TerraRockio — controller configuration (geometry, servo map, limits).
// Clean-room original. Proprietary — see /LICENSE.
//
// Everything you'd tune for a new chassis or after calibration lives here.
// Values marked TUNE are placeholders to confirm against the final CAD / on the
// bench; the math elsewhere reads from these so you never touch logic to retune.
#pragma once
#include <Arduino.h>

namespace config {

// ---- Leg geometry (mm) — Design B, TUNE to final CAD --------------------
constexpr float COXA_LEN   = 30.0f;   // coxa pivot -> femur pivot (horizontal)
constexpr float FEMUR_LEN  = 60.0f;   // Design B femur
constexpr float TIBIA_LEN  = 80.0f;   // Design B tibia

// ---- Body layout --------------------------------------------------------
constexpr int   NUM_LEGS   = 6;
constexpr int   DOF        = 3;
constexpr int   NUM_SERVOS = NUM_LEGS * DOF;   // 18

// Coxa mounts: 6 legs evenly at 60deg on a circle (symmetric hexagon).
constexpr float MOUNT_RADIUS      = 60.0f;     // ~120mm coxa circle (Design B)
constexpr float MOUNT_ANGLE_DEG[NUM_LEGS] = {30, 90, 150, 210, 270, 330};

// ---- Default stance (the pose it holds standing still) ------------------
constexpr float STANCE_RADIUS = 90.0f;  // foot horizontal dist from coxa pivot
                                        // == Design B's ~90mm moment arm
constexpr float STANCE_HEIGHT = 75.0f;  // body clearance; clears 50mm drip line

// ---- Gait ---------------------------------------------------------------
constexpr float STEP_HEIGHT   = 30.0f;  // foot lift during swing (mm)
constexpr float GAIT_HZ       = 1.2f;   // full gait cycles per second
constexpr float CONTROL_HZ    = 50.0f;  // servo update rate

// ---- Dynamixel bus ------------------------------------------------------
constexpr float DXL_PROTOCOL  = 2.0f;
constexpr long  DXL_BAUD      = 1000000;  // 1 Mbps
constexpr int   DXL_DIR_PIN   = 5;        // half-duplex direction pin -> transceiver
// On ESP32 set the bus UART RX/TX in servo_bus.cpp (Serial1).

// Servo IDs, indexed [leg*3 + joint], joint 0=coxa 1=femur 2=tibia.
constexpr uint8_t SERVO_ID[NUM_SERVOS] = {
  11, 12, 13,   // leg 0
  21, 22, 23,   // leg 1
  31, 32, 33,   // leg 2
  41, 42, 43,   // leg 3
  51, 52, 53,   // leg 4
  61, 62, 63,   // leg 5
};

// Per-servo direction (+1/-1) — flip if a joint moves the wrong way. TUNE.
constexpr int8_t SERVO_DIR[NUM_SERVOS] = {
  1, 1, 1,  1, 1, 1,  1, 1, 1,
  1, 1, 1,  1, 1, 1,  1, 1, 1,
};

// Per-servo zero offset (deg) so a commanded 0 = mechanical neutral. TUNE on bench.
constexpr float SERVO_OFFSET_DEG[NUM_SERVOS] = {
  0, 0, 0,  0, 0, 0,  0, 0, 0,
  0, 0, 0,  0, 0, 0,  0, 0, 0,
};

// XL330: 0..4095 ticks over 360deg, 2048 = centre (180deg).
constexpr float TICKS_PER_DEG = 4096.0f / 360.0f;
constexpr int   TICK_CENTER   = 2048;

// ---- Safety -------------------------------------------------------------
constexpr float MAX_TEMP_C        = 70.0f;  // XL330 shutdown well above this
constexpr float LOW_VOLT_CUTOFF_V = 9.6f;   // 3S empty (~3.2V/cell) — protect pack
constexpr uint32_t HEARTBEAT_TIMEOUT_MS = 1500; // no Pi contact -> stop walking

// ---- Helpers ------------------------------------------------------------
constexpr float DEG2RAD = PI / 180.0f;
constexpr float RAD2DEG = 180.0f / PI;

} // namespace config
