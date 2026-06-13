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

## Flashing

The Alchemy Lab runs a custom bootloader (`DaisyBootloader-AlchemyLabV2`)
that serves DFU over the front-panel USB-C port.  Connect that port, then put
the module in update mode: during the ~2 s window after power-on — the LED
rings spin a warm-white comet — press or hold **B3.**  The rings switch to a slow breathe, and the module stays in DFU mode until it's flashed or reset.  Then:

```sh
make program-dfu
```

You can also use the [Hermetic Modular Web Programmer](https://hermeticmodular.com/program) straight from the browser.

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


## License

MIT — see [LICENSE](LICENSE).  libDaisy is independently MIT-licensed by
Electrosmith.
