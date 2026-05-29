// TerraRockio — inverse kinematics implementation. Clean-room. Proprietary.
#include "kinematics.h"
#include <math.h>

namespace {
// Foot point relative to a leg's coxa pivot, rotated so local +x points
// straight out along that leg's mount direction.
void toLegLocal(int leg, const Vec3& footBody, float& lx, float& ly, float& lz) {
  const float yaw = config::MOUNT_ANGLE_DEG[leg] * config::DEG2RAD;
  const float mx  = config::MOUNT_RADIUS * cosf(yaw);
  const float my  = config::MOUNT_RADIUS * sinf(yaw);
  const float dx  = footBody.x - mx;
  const float dy  = footBody.y - my;
  lx =  cosf(yaw) * dx + sinf(yaw) * dy;   // outward
  ly = -sinf(yaw) * dx + cosf(yaw) * dy;   // sideways
  lz =  footBody.z;                         // up(+)/down(-)
}
} // namespace

LegAngles kinematics::solveLeg(int leg, const Vec3& footBody) {
  float lx, ly, lz;
  toLegLocal(leg, footBody, lx, ly, lz);

  LegAngles out;
  // Coxa yaw aims the leg plane at the foot.
  out.coxa = atan2f(ly, lx);

  // Reduce to a 2-link planar problem (femur+tibia) in that plane.
  const float horiz = sqrtf(lx * lx + ly * ly) - config::COXA_LEN;
  const float d     = sqrtf(horiz * horiz + lz * lz);   // hip->foot distance

  const float reach = config::FEMUR_LEN + config::TIBIA_LEN;
  const float minReach = fabsf(config::FEMUR_LEN - config::TIBIA_LEN);
  float dc = d;
  if (d > reach || d < minReach) {            // outside workspace
    out.reachable = false;
    dc = fminf(fmaxf(d, minReach + 0.01f), reach - 0.01f);  // clamp for safe math
  }

  const float a1 = atan2f(lz, horiz);         // angle of hip->foot line
  const float a2 = acosf((config::FEMUR_LEN * config::FEMUR_LEN + dc * dc -
                          config::TIBIA_LEN * config::TIBIA_LEN) /
                         (2.0f * config::FEMUR_LEN * dc));
  out.femur = a1 + a2;                         // femur up from horizontal

  const float knee = acosf((config::FEMUR_LEN * config::FEMUR_LEN +
                            config::TIBIA_LEN * config::TIBIA_LEN - dc * dc) /
                           (2.0f * config::FEMUR_LEN * config::TIBIA_LEN));
  out.tibia = knee - PI;                       // 0 == straight leg
  return out;
}

Vec3 kinematics::applyBodyPose(const Vec3& foot, const Vec3& t, const Vec3& r) {
  // Rotate the foot about the body centre (Rz*Ry*Rx) then translate.
  const float cr = cosf(r.x), sr = sinf(r.x);   // roll
  const float cp = cosf(r.y), sp = sinf(r.y);   // pitch
  const float cy = cosf(r.z), sy = sinf(r.z);   // yaw

  // Combined rotation matrix rows.
  const float x = foot.x, y = foot.y, z = foot.z;
  Vec3 o;
  o.x = cy * cp * x + (cy * sp * sr - sy * cr) * y + (cy * sp * cr + sy * sr) * z + t.x;
  o.y = sy * cp * x + (sy * sp * sr + cy * cr) * y + (sy * sp * cr - cy * sr) * z + t.y;
  o.z = -sp * x + cp * sr * y + cp * cr * z + t.z;
  return o;
}
