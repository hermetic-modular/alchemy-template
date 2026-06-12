# alchemy-template

The recommended starting point for your own [Hermetic Modular Alchemy
Lab](https://hermeticmodular.com/modules/alchemy-lab) module firmware.

This is the [Alchemy SDK](https://github.com/hermetic-modular/alchemy-sdk)'s
`stereo_eq` example — a dual mono three-band EQ that opts into the full
framework stack (pagination, pot catch, param lock, CV routing, presets,
settings, LED animations) — packaged as a standalone project that vendors
the Alchemy SDK and libDaisy as git submodules and builds with the standard
Daisy `make` workflow.  Clone it, build it, flash it, then gut `src/` and
make it yours.

## What's inside

```
├── Makefile             standard Daisy Makefile (libDaisy core underneath)
├── src/                 the firmware — this is the part you edit
│   ├── stereo_eq.cpp        hardware wiring, pages, knobs, CV, presets
│   ├── stereo_eq_dsp.*      pure DSP (three-band biquad EQ per channel)
│   └── stereo_eq_palette.h  LED color palettes
└── lib/
    ├── alchemy-sdk/     Alchemy framework + board support   (submodule)
    └── libDaisy/        Electrosmith Daisy library           (submodule)
```

## Requirements

- `git`
- `make`
- `arm-none-eabi-gcc`
- `dfu-util`

No CMake or Ninja needed — this template uses the plain Daisy Make build.

Ubuntu / Debian:

```sh
sudo apt install git make gcc-arm-none-eabi dfu-util
```

macOS (Homebrew):

```sh
brew install git make dfu-util
brew install --cask gcc-arm-embedded
```

## Getting started

```sh
git clone --recurse-submodules https://github.com/hermetic-modular/alchemy-template.git my-module
cd my-module

make libdaisy    # build libDaisy once after cloning
make             # build the firmware → build/stereo_eq.bin
```

<details>
<summary>Smaller download (optional)</summary>

`--recurse-submodules` also pulls the Alchemy SDK's own nested copy of
libDaisy, which this template doesn't use.  To skip it:

```sh
git clone https://github.com/hermetic-modular/alchemy-template.git my-module
cd my-module
git submodule update --init lib/alchemy-sdk
git submodule update --init --recursive lib/libDaisy
```

</details>

## Board versions

The Alchemy Lab exists in two hardware revisions.  Firmware written against
the unified `alchemy::AlchemyLab` board class (as `src/stereo_eq.cpp` is)
builds for either — pick at build time:

```sh
make             # V2 board (current hardware, the default)
make BOARD=v1    # V1 board
```

Switching `BOARD` automatically rebuilds from scratch — the two board
support packages intentionally share class and file names, so their objects
must never mix in one build tree.

## Flashing

Put the module in DFU mode: hold **BOOT**, tap **RESET**, release **BOOT**.
The Daisy bootloader also gives you a short DFU window right after a
reset — press **RESET** during the bootloader's "breathing" LED animation
to extend it.  Then:

```sh
make program-dfu
```

This uploads `build/stereo_eq.bin` to QSPI flash at `0x90040000`, where the
stock Daisy bootloader expects it (it copies the image to SRAM on boot and
runs it from there).

## Make it yours

1. **Rename the firmware** — change `TARGET` at the top of the
   [`Makefile`](Makefile) (this names the `.bin`), and rename the `src/`
   files to taste, updating `CPP_SOURCES` to match.
2. **Bring your own DSP** — replace `stereo_eq_dsp.*` and rewire the knobs,
   pages, and CV matrix in `stereo_eq.cpp`.  Every framework feature is an
   explicit constructor call; delete what you don't want.
3. **Add source files** — append them to `CPP_SOURCES` in the Makefile.
   One caveat from the underlying Daisy build: object files are flattened
   into `build/` by basename, so two sources can't share a filename even in
   different directories.
4. **Learn the SDK** — the framework headers live in
   `lib/alchemy-sdk/framework/include/alchemy/`, and the SDK's
   [`examples/`](https://github.com/hermetic-modular/alchemy-sdk/tree/main/examples)
   show other usage styles (the `kick` example is a minimal-opt-in
   contrast to this template).

### Updating the vendored libraries

```sh
git -C lib/alchemy-sdk pull origin main
git add lib/alchemy-sdk && git commit -m "Bump alchemy-sdk"
```

The pinned libDaisy commit matches the one the Alchemy SDK itself vendors
and tests against; if you bump one, consider bumping the other to match.

## How the build hangs together

The [`Makefile`](Makefile) is a normal Daisy project Makefile — it sets
`CPP_SOURCES`, includes `lib/libDaisy/core/Makefile`, and inherits all the
standard targets (`all`, `clean`, `program-dfu`, …).  On top of that it:

- compiles the Alchemy SDK straight from the submodule (the framework
  sources plus the board support package selected by `BOARD`);
- builds in `APP_TYPE = BOOT_SRAM` for the Daisy bootloader, using the
  SDK's vendored linker script and supplementary linker fragments;
- sets `-std=gnu++17`, which the SDK requires.

### Linker warnings you can ignore

Recent binutils (≥ 2.45, bundled with current Arm GNU toolchains) changed
how `INSERT` linker-script fragments resolve, so the fragment is passed
before the primary script and the link emits a benign pair of warnings
about the `RAM_D2` memory region (referenced in one script, declared in
the other).  The layout is correct — the SDK's `.ahb_sram_bss` section
lands at the base of RAM_D2 with the heap after it; check with
`arm-none-eabi-readelf -S build/stereo_eq.elf` if you're curious.  A
`LOAD segment with RWX permissions` warning is likewise expected for
bare-metal Daisy images on new toolchains.

## License

MIT — see [LICENSE](LICENSE).  libDaisy is independently MIT-licensed by
Electrosmith.
