# Energy Budget — Design B

## Battery

```
3S 18650 = 3 cells × 3.6 V nominal × 3000 mAh
         ≈ 32 Wh,  ~141 g
```

## Average power from the duty cycle

The robot spends most of its time standing still and looking, not walking:

| Mode | Share of time | Power |
|---|---:|---:|
| Walking | 10% | ~30 W |
| Stationary / observing | 90% | ~6–7 W |

```
Weighted average = 0.10 × 30 W + 0.90 × 6.5 W ≈ 9 W
```

## Runtime

```
Runtime = 32 Wh × 0.85 (usable capacity) × 0.88 (converter efficiency) / 9 W
        ≈ 2.6 h   (ideal)
```

Derate **~30%** for cold greenhouse nights, battery aging, and current spikes:

```
Real-world ≈ 1.8–2 h
```

## Where this is solid vs. shaky

| Confidence | What |
|---|---|
| ✅ Datasheet-grade | Battery cell energy (32 Wh) |
| 🟡 Estimate (±20%) | Average power, runtime |
| 🔬 Needs bench validation | **Servo holding current** and the **90%-stationary duty cycle** |

## The two assumptions that can break this

1. **Servo holding current.** The entire budget rests on the servos drawing very little while the robot stands still. The estimate assumes ~10 mA holding. Some servos draw 30–50 mA holding instead — and at 18 servos that difference could *halve* the runtime. This must be measured on the bench before the runtime number is trusted. (See [validation-plan.md](validation-plan.md).)

2. **Duty cycle.** If the robot actually walks far more than 10% of the time, average power climbs and runtime drops proportionally. This is the single biggest unknown in the whole design — and one phone call with a greenhouse manager (how big are the rows, how often does it need to reposition?) settles it.
