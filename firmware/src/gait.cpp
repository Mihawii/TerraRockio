// TerraRockio — tripod gait implementation. Clean-room. Proprietary.
#include "gait.h"
#include <math.h>

namespace {
using config::NUM_LEGS;

Vec3  g_nominal[NUM_LEGS];          // default planted foot positions (body frame)
float g_vx = 0, g_vy = 0, g_omega = 0;
float g_phase = 0.0f;               // 0..1 over a full gait cycle

constexpr float DUTY = 0.5f;        // half cycle stance, half swing (tripod)
} // namespace

void gait::setStance() {
  for (int i = 0; i < NUM_LEGS; ++i) {
    const float yaw = config::MOUNT_ANGLE_DEG[i] * config::DEG2RAD;
    const float r   = config::MOUNT_RADIUS + config::STANCE_RADIUS;
    g_nominal[i].x = r * cosf(yaw);
    g_nominal[i].y = r * sinf(yaw);
    g_nominal[i].z = -config::STANCE_HEIGHT;
  }
}

void gait::setWalk(float vx, float vy, float omega) {
  g_vx = vx; g_vy = vy; g_omega = omega;
}

void gait::advance(float dPhase) {
  g_phase += dPhase;
  while (g_phase >= 1.0f) g_phase -= 1.0f;
}

bool gait::standing() {
  return g_vx == 0.0f && g_vy == 0.0f && g_omega == 0.0f;
}

void gait::footTargets(Vec3 out[NUM_LEGS]) {
  const float halfX = g_vx * 0.5f;
  const float halfY = g_vy * 0.5f;
  const float halfR = g_omega * 0.5f;

  for (int i = 0; i < NUM_LEGS; ++i) {
    const Vec3& base = g_nominal[i];
    // Tripod B (odd legs) is half a cycle out of phase with tripod A.
    float p = g_phase + ((i & 1) ? 0.5f : 0.0f);
    if (p >= 1.0f) p -= 1.0f;

    float s;        // stride progress, +1 (front) .. -1 (back)
    float lift = 0.0f;
    if (p < DUTY) {                       // STANCE: foot pushes body forward
      s = 1.0f - (p / DUTY) * 2.0f;       //  +1 -> -1, foot on ground
    } else {                              // SWING: lift and return to front
      const float sp = (p - DUTY) / (1.0f - DUTY);   // 0..1
      s    = -1.0f + sp * 2.0f;
      lift = config::STEP_HEIGHT * sinf(PI * sp);     // smooth arc
    }

    // Linear stride component.
    const float ox = halfX * s;
    const float oy = halfY * s;
    // Turn component: rotate the planted point about the body centre.
    const float ang = halfR * s;
    const float rx = base.x * cosf(ang) - base.y * sinf(ang) - base.x;
    const float ry = base.x * sinf(ang) + base.y * cosf(ang) - base.y;

    out[i].x = base.x + ox + rx;
    out[i].y = base.y + oy + ry;
    out[i].z = base.z + lift;
  }
}
