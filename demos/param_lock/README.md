# Parameter-lock demos

Five firmware variants demonstrate `ParamLock` thresholds and LED styles on Alchemy Lab V2. Each outputs silence. J2 supplies an optional external clock.

| `DEMO=` | Arm / clear threshold | Default exit | Recording pip | Playback pip |
| --- | --- | --- | --- | --- |
| `original` | 3% / 3% | Latch | solid red | solid green |
| `new_default` | 0.5% / 3% | Return | solid red | blinking red |
| `breathe` | 0.5% / 3% | Return | solid violet | breathing violet |
| `loop_pulse` | 0.5% / 3% | Return | solid red | red flash at each loop restart |
| `sweep` | 0.5% / 3% | Return | red pip follows the write head | red pip follows the play head |

`original` reproduces the v0.10.0 configuration. `new_default` uses the SDK defaults introduced in v0.11.0. All variants build against the SDK pinned by this template.

At startup, rings P1 through PN light white for one second. N is the variant's row number in the table.

## Controls

- P1 through P6 are normalized values shown as level arcs. The top row is cyan, the middle row orange, and the bottom row green.
- Hold B1 and turn a pot to record its movement. Release B1 to loop. Hold B1 and move a looping pot to clear its lock.
- J2 accepts 24 pulses per quarter note. Change `kClockPpqn` in `lock_demo.cpp` for another division: 4 for sixteenth notes or 1 for quarter notes.
- Hold B2 and B3 to enter settings. P1 controls brightness, P3 and P4 control presets, P5 selects Free or Clocked lock timing, and P6 selects Return or Latch. A saved boot preset can override these defaults.
- HostLink exposes the controls and manual in the web programmer and supports `program-live`.

## Build

Build libDaisy once from the template root:

```sh
make libdaisy
```

Then build a variant from the root:

```sh
make DEMO=sweep
```

Or build all five from this directory:

```sh
make
```

Binaries are written to `demos/param_lock/build/lock_demo_<variant>.bin`, relative to the template root. `make clean` in this directory removes all demo build output.

## Flash

With HostLink firmware running, from this directory or the template root:

```sh
make DEMO=original program-live
```

For a module already in DFU mode:

```sh
make DEMO=original program-dfu
```

These commands require a variant. `DEMO=lock_demo_breathe` and `DEMO=lock_demo_breathe.bin` also select `breathe`. From the template root, commands without `DEMO=` continue to build and flash `stereo_eq`.

## Source files

- `lock_demo.cpp`: hardware, controls, clock, settings, HostLink, and startup identifier.
- `lock_demo.h`: the `LockDemoSpec` supplied by each variant.
- `demo_<variant>.cpp`: module identity, manual text, and lock configuration.
