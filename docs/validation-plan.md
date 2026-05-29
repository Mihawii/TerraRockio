# Validation Plan

The design is internally consistent, but four numbers are estimates that the whole thing leans on. Until these are measured on a bench, treat the spec as *predicted*, not *proven*. Listed in priority order — the first two can each break the design on their own.

## 1. Servo holding current 🔬🔬 (biggest energy risk)

**Why it matters:** the entire [energy budget](energy-budget.md) assumes servos draw ~10 mA while standing still (90% of the time). If they actually draw 30–50 mA holding, runtime roughly halves.

**How to measure:**
- Put one XL330 under a representative static load, command a hold, and read current draw — either from the Dynamixel current telemetry register or an inline meter.
- Repeat at the worst-case holding torque (the 3.0 kgf·cm stance load).
- Multiply by 18 and re-run the energy budget.

**Pass criterion:** total holding draw consistent with the ≤9 W average, i.e. runtime ≥ ~1.8 h.

## 2. Real horizontal moment arm 🔬🔬 (biggest torque risk)

**Why it matters:** torque scales linearly with the arm. The [design spec](design-spec.md) assumes ~90 mm in the real stance pose; if the actual pose is longer, required torque rises and margin shrinks.

**How to measure:**
- Build/print one leg, set it to the intended stance pose, and measure the horizontal distance from the loaded joint axis to the foot contact point.
- Recompute required torque and re-check margin against 5.3 kgf·cm stall.

**Pass criterion:** worst dynamic torque stays under stall with margin (target ≥ 1.2×).

## 3. Peak transient torque under the real gait 🔬

**Why it matters:** the 4.5 kgf·cm "worst dynamic instant" is a hand estimate. The actual peak depends on the specific gait trajectory and timing.

**How to measure:**
- Run the tripod gait on a single instrumented leg (or full robot), log per-servo load/current telemetry over many cycles, and find the true peak.
- If peaks exceed comfortable margin, retune the gait (slower step transfer, lower body, weight-shift timing) — i.e. "train gaits that don't hit that peak."

**Pass criterion:** logged peak load stays within stall margin across normal operation.

## 4. Duty cycle assumption 🔬 (cheapest to resolve)

**Why it matters:** the energy budget assumes ~90% stationary / ~10% walking. If the robot must walk far more, average power climbs and runtime drops.

**How to resolve:**
- **One phone call with a greenhouse manager.** Row lengths, plant spacing, how often it must reposition, how long it dwells per inspection point. No hardware needed.
- This is the single biggest unknown in the whole design and the cheapest to close.

**Pass criterion:** observed/described walking fraction ≤ ~10%, or the budget is updated to reality.

---

## Summary

| # | Item | Risk | Cost to resolve |
|---|---|---|---|
| 1 | Servo holding current | Could halve runtime | One servo + a meter |
| 2 | Real moment arm | Direct hit to torque margin | One printed leg |
| 3 | Peak transient torque | Margin at dynamic peak | Instrumented gait run |
| 4 | Duty cycle | Scales runtime | **One phone call** |
