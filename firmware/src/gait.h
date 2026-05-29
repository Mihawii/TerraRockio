// TerraRockio — tripod gait generator. Clean-room original. Proprietary.
//
// Produces foot-contact targets (body frame) for the current phase. Two
// tripods {0,2,4} and {1,3,5} are kept exactly half a cycle apart, so three
// feet are always planted — the robot can be stopped at any phase and stand
// rock-steady (the imaging requirement).
#pragma once
#include "kinematics.h"

namespace gait {

void setStance();                                   // build nominal foot points
void setWalk(float vx, float vy, float omega);      // mm/cycle, mm/cycle, rad/cycle
void advance(float dPhase);                         // step the gait phase forward
bool standing();                                    // true when phase-frozen & no walk
void footTargets(Vec3 out[config::NUM_LEGS]);       // current targets, body frame

} // namespace gait
