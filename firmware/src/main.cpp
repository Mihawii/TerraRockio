// TerraRockio — realtime controller (ESP32). Clean-room original. Proprietary.
//
// The "spinal cord": owns the fixed-rate gait + balance loop so the body stays
// stable regardless of what the Pi (the "brain") is doing. The Pi sends goals
// over UART; this never blocks on it.
//
// STATUS: v0, bench-untested. Geometry/offsets in config.h are placeholders —
// bring up one leg first, calibrate SERVO_DIR/OFFSET, then enable all six.
//
// Pi link protocol (Serial2, newline-terminated ASCII):
//   W <vx> <vy> <omega>      walk    (mm/cycle, mm/cycle, rad/cycle)
//   B <tx> <ty> <tz> <rx> <ry> <rz>  body pose (mm, rad) — aim / hold still
//   U | D                    stand up / sit down
//   S                        stop (freeze on a stable tripod)
//   H                        heartbeat (must arrive < HEARTBEAT_TIMEOUT_MS)
// Replies: "TLM <volt> <maxTemp> <maxCurrent> <ikErr> <healthy>"
#include "config.h"
#include "kinematics.h"
#include "gait.h"
#include "servo_bus.h"

namespace {
HardwareSerial& Pi = Serial2;          // brain link
char  line[96];
int   lineLen = 0;

Vec3  bodyTrans, bodyRot;              // commanded body pose
uint32_t lastHeartbeat = 0;
uint32_t lastControlUs = 0;
uint32_t lastTlmMs = 0;
bool  ikError = false;

// --- IMU balance hook ----------------------------------------------------
// TODO: read a BNO055/MPU6050 here and return small roll/pitch trims so the
// body self-levels on uneven greenhouse floors. Stubbed to zero for now.
void imuBalanceTrim(Vec3& rotTrim) { rotTrim.x = 0; rotTrim.y = 0; }

void handleLine(char* s) {
  char* tok = strtok(s, " ");
  if (!tok) return;
  switch (tok[0]) {
    case 'W': {
      float vx = atof(strtok(nullptr, " "));
      float vy = atof(strtok(nullptr, " "));
      float w  = atof(strtok(nullptr, " "));
      gait::setWalk(vx, vy, w);
      break;
    }
    case 'B': {
      bodyTrans.x = atof(strtok(nullptr, " "));
      bodyTrans.y = atof(strtok(nullptr, " "));
      bodyTrans.z = atof(strtok(nullptr, " "));
      bodyRot.x   = atof(strtok(nullptr, " "));
      bodyRot.y   = atof(strtok(nullptr, " "));
      bodyRot.z   = atof(strtok(nullptr, " "));
      break;
    }
    case 'S': gait::setWalk(0, 0, 0); break;
    case 'U': servobus::torque(true);  break;
    case 'D': servobus::torque(false); break;
    case 'H': lastHeartbeat = millis(); break;
  }
}

void readPi() {
  while (Pi.available() > 0) {
    char c = Pi.read();
    if (c == '\n' || c == '\r') {
      if (lineLen > 0) { line[lineLen] = '\0'; handleLine(line); lineLen = 0; }
    } else if (lineLen < (int)sizeof(line) - 1) {
      line[lineLen++] = c;
    }
  }
}

void sendTelemetry(const ServoTelemetry& t) {
  float maxTemp = 0, maxCur = 0;
  for (int i = 0; i < config::NUM_SERVOS; ++i) {
    if (t.temp_C[i]    > maxTemp) maxTemp = t.temp_C[i];
    if (t.current_mA[i] > maxCur) maxCur = t.current_mA[i];
  }
  Pi.printf("TLM %.2f %.1f %.0f %d %d\n", t.voltage_V, maxTemp, maxCur,
            ikError ? 1 : 0, servobus::healthy() ? 1 : 0);
}
} // namespace

void setup() {
  Serial.begin(115200);                                  // USB debug
  Pi.begin(115200, SERIAL_8N1, /*RX*/25, /*TX*/26);      // brain link
  gait::setStance();
  if (!servobus::begin()) Serial.println("WARN: not all servos responded");
  lastHeartbeat = millis();
  lastControlUs = micros();
}

void loop() {
  readPi();

  // Failsafe: lost the brain -> stop walking, hold a stable tripod.
  if (millis() - lastHeartbeat > config::HEARTBEAT_TIMEOUT_MS) gait::setWalk(0, 0, 0);

  // Fixed-rate control tick.
  const uint32_t now = micros();
  const uint32_t periodUs = (uint32_t)(1e6f / config::CONTROL_HZ);
  if (now - lastControlUs < periodUs) return;
  lastControlUs += periodUs;

  // Only advance the gait phase while actually walking; frozen = steady tripod.
  if (!gait::standing()) gait::advance(config::GAIT_HZ / config::CONTROL_HZ);

  Vec3 targets[config::NUM_LEGS];
  gait::footTargets(targets);

  Vec3 rotTrim; imuBalanceTrim(rotTrim);
  Vec3 rot = { bodyRot.x + rotTrim.x, bodyRot.y + rotTrim.y, bodyRot.z };

  LegAngles legs[config::NUM_LEGS];
  ikError = false;
  for (int i = 0; i < config::NUM_LEGS; ++i) {
    Vec3 posed = kinematics::applyBodyPose(targets[i], bodyTrans, rot);
    legs[i] = kinematics::solveLeg(i, posed);
    if (!legs[i].reachable) ikError = true;
  }

  if (!ikError && servobus::healthy()) servobus::writeLegAngles(legs);

  // Health sampling + telemetry to the brain (~10 Hz).
  ServoTelemetry tlm;
  servobus::pollTelemetry(tlm);
  if (millis() - lastTlmMs > 100) { sendTelemetry(tlm); lastTlmMs = millis(); }
}
