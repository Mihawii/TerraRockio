# Leg design — the beginner-friendly flat-bracket leg

One leg, built from **3 XL330 servos + 3 simple printed plates.** Two plates are dead-flat; one is an L. You make these once, then mirror/repeat for all 6 legs. No fancy curves, no scaling.

## The leg, side-on

```
   BODY plate
      │
  ┌───────────┐
  │ COXA servo│  bolted flat to the body, output shaft points UP
  └───────────┘  → swings the whole leg left / right
      │
   [L-bracket]   the one corner piece (turns 90°)
      │
  ┌───────────┐
  │FEMUR servo│  output shaft horizontal
  └───────────┘  → lifts the leg up / down
      │
  [ Femur link ] flat plate, ~60 mm long
      │
  ┌───────────┐
  │TIBIA servo│  output shaft horizontal
  └───────────┘  → bends the knee
      │
  [ Tibia link ] flat plate, ~80 mm long, ends in a foot
      ▼
     foot
```

3 servos = the 3 joints. 3 plates = the bits between them. That's a whole leg.

## ⭐ The one trick that makes this easy: don't measure holes — copy them

You already imported the XL330 into Fusion. **Every screw hole you need is on that model.** So you never measure or guess hole positions — you trace them off the real part:

1. Start a sketch on the face where the plate will sit (`Create Sketch`, click the servo's flat face).
2. Press **P** (Project). Click the servo's screw holes and its output-horn holes — their circles drop into your sketch *exactly* where they really are.
3. Draw your plate outline (a rectangle) around those circles.
4. **E** (Extrude) the plate ~3 mm thick.
5. Use those projected hole-circles to **Extrude → Cut** through the plate.

A perfect-fitting plate, and you never typed a single hole coordinate. That's the whole skill, repeated 3 times.

## The 3 parts to make

| Part | Shape | Size | Connects |
|---|---|---|---|
| **Femur link** | flat rectangle | ~60 × 20 × 3 mm | femur servo's spinning horn → tibia servo's body |
| **Tibia link** | flat rectangle + a rounded foot tip | ~80 × 20 × 3 mm | tibia servo's horn → the ground |
| **L-bracket** | an "L" (one sketch, one extrude) | ~25 mm per side, 3 mm | coxa servo's horn (flat) → femur servo's body (upright) |

Every hole on every part is traced from the real servo using the trick above. The only numbers *you* choose are the **lengths** (60 mm and 80 mm) and the **thickness** (3 mm).

## Fixed facts (so you're not guessing)

- **Servo body:** 20 × 34 × 26 mm. **Screws:** M2.
- **The spinning side** of each joint bolts to the servo's **horn** (the output disc). **The still side** bolts to the next servo's **case**. Get this right per joint and the leg articulates correctly.
- **Plate thickness:** 3 mm is a good start (stiff enough in PETG, still light).
- **Lengths:** femur 60 mm, tibia 80 mm (centre-of-joint to centre-of-joint). These are the Design B numbers — if you change them, tell me and I update two lines in the firmware (`FEMUR_LEN`, `TIBIA_LEN` in `firmware/include/config.h`) so the walking math matches your real leg.

## Build order (screwdriver, no CAD)

1. **Coxa servo** → bolt flat to the body plate, shaft up.
2. **L-bracket** → bolt its flat face to the coxa servo's horn; the upright face now sticks up.
3. **Femur servo** → bolt its case to the L-bracket's upright face.
4. **Femur link** → bolt one end to the femur servo's horn, the other end to the **tibia servo's** case.
5. **Tibia link** → bolt one end to the tibia servo's horn; the foot end touches the ground.
6. Repeat ×6. Three legs are mirror images of the other three (in Fusion: select the parts → **Mirror**).

## Expect to iterate (this is normal)

Your first plate probably won't fit perfectly. That's the loop everyone runs:
**print it → hold it against the real servo → nudge a hole or length → reprint.**
A small plate prints in ~15 min, so this is cheap. Get **one leg** feeling solid before you commit to printing six.

## Print settings (PETG)

- 3 mm thick (already in the design), 0.2 mm layers.
- 4 walls + ~40% infill on the links (they carry load); the foot tip can be denser.
- A soft (TPU) or rubber cap on the foot tip helps grip — optional for v1.
