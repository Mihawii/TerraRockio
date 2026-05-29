# Design Specification — Design B

This is the current, recommended design. It targets a ~1 kg robot whose legs *actually move well* — sized so the servos have comfortable torque margin in a normal stance and only run thin at a brief dynamic peak (which the gait is trained to avoid).

## 1. Configuration

- **Hexapod**, 6 legs at 60° spacing around the body.
- **3 DOF per leg** (coxa / femur / tibia) → **18 actuated joints** total.
- **Gait:** tripod — three alternating feet always planted, so the robot can stop and hold a stable, vibration-free pose for imaging.

## 2. Actuator

**Dynamixel XL330-M288** (×18)

| Property | Value | Source |
|---|---|---|
| Mass | ~23 g | datasheet |
| Stall torque | ~5.3 kgf·cm (~0.52 N·m) | datasheet |
| Interface | TTL serial bus (daisy-chainable) | datasheet |

Smart serial servos (not hobby PWM) — they daisy-chain on one bus, report position/load/temperature/current, and are addressable individually, which keeps wiring sane across 18 joints and makes the holding-current and load telemetry in the validation plan possible.

## 3. Geometry / proportions

| Dimension | Value |
|---|---|
| Chassis (L × W × H) | 130 × 100 × 40 mm |
| Coxa mount circle | ~120 mm diameter (spacing keeps swinging legs from colliding) |
| Femur | 60 mm |
| Tibia | 80 mm |
| Standing body clearance | ~75 mm |
| Tip-to-tip span (standing) | ~290 mm |

The 75 mm body clearance is deliberate: it clears a **50 mm drip line** — the most common obstacle on a greenhouse floor — with margin.

## 4. Torque analysis

Sizing is done for the tripod gait, where the body weight is carried by **3 stance legs** at any instant.

**Steady stance (the 90%-of-the-time case):**
```
Per-leg vertical load  = (1.0 kg × 9.81 m/s²) / 3 legs = 3.27 N
Effective horizontal moment arm in stance ≈ 90 mm
Joint torque required  = 3.27 N × 0.090 m = 0.29 N·m = 3.0 kgf·cm
```

**Worst dynamic instant** (during a step transfer, one leg briefly carries ~50% of the body):
```
Load   = 0.50 × 9.81 N = 4.9 N
Torque = 4.9 N × 0.090 m = 0.44 N·m = 4.5 kgf·cm
```

**Margins against the 5.3 kgf·cm stall:**

| Case | Required | Margin |
|---|---|---|
| Steady stance | 3.0 kgf·cm | **1.8×** |
| Worst dynamic instant | 4.5 kgf·cm | **1.2×** |

Comfortable for holding; thin only at the absolute peak — acceptable because the gait is trained not to hit that peak. This is the sweet spot: enough headroom that the legs move briskly rather than straining at the limit.

> ⚠️ The 90 mm arm is an estimate of the *real* stance pose, not a measured value. It feeds the torque directly, so it's first on the [validation plan](validation-plan.md).

## 5. Mass budget

~1,006 g total. See [`bill-of-materials.md`](bill-of-materials.md) for the line items.

## 6. Stability

The center of mass must stay inside the support triangle formed by the 3 stance feet. With a **centered, low battery** and splayed legs, this holds with comfortable margin.

> The single placement that matters: keep the battery **low and central**. Everything else in the layout has slack; this one doesn't.

## 7. Material

**PETG**, 3D-printed — chosen over PLA for tolerance to greenhouse heat and humidity. Print settings per part: [`../hardware/README.md`](../hardware/README.md).
