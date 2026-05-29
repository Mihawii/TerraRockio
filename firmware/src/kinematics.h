// TerraRockio — inverse kinematics (3-DOF leg + body pose).
// Clean-room original, implemented from the standard published hexapod IK
// (law-of-cosines leg solver; see oscarliang.com hexapod IK writeup for the
// geometry). Proprietary — see /LICENSE.
#pragma once
#include "config.h"

struct Vec3 {
  float x = 0, y = 0, z = 0;
};

// Joint angles for one leg, radians. reachable=false if the target is outside
// the leg's workspace (caller should hold the previous pose).
struct LegAngles {
  float coxa = 0, femur = 0, tibia = 0;
  bool  reachable = true;
};

namespace kinematics {

// Solve one leg for a foot contact point given in the BODY frame (mm).
LegAngles solveLeg(int leg, const Vec3& footBody);

// Re-express a foot point after moving the body by `translation` (mm) and
// rotating it by `rotation` (rad, x=roll y=pitch z=yaw). Used for hold-still
// aiming and IMU balance trims.
Vec3 applyBodyPose(const Vec3& foot, const Vec3& translation, const Vec3& rotation);

} // namespace kinematics
