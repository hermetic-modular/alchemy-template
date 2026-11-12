/**
 * lock_demo.cpp : visual-only ParamLock demo, shared by every variant.
 *
 * There is no DSP.  The audio callback outputs silence and exists only so
 * the V2 audio shim keeps feeding J2 to its trigger detector, which is
 * where the clock comes in.
 *
 *   - Six plain 0..1 params on one page, Level arcs: top row cyan,
 *     middle row orange, bottom row green.
 *   - B1: hold and turn a pot to record a lock on it; release to loop.
 *     Hold B1 and nudge a looping pot to clear it.
 *   - J2: external clock → ClockFollower → MusicalClock.  Clocked lock
 *     mode snaps loop lengths to it.  See kClockPpqn.
 *   - Settings (hold B2 + B3): the SDK default stack : P1 brightness,
 *     P3–P4 presets, P5 Free/Clocked, P6 Return/Latch.  The variant picks
 *     what P6 boots as: Latch for the v0.10.0 original, Return otherwise.
 *   - HostLink, so `make DEMO=<variant> program-live` can flash it.
 *   - Boot identifier: rings P1..PN light white for one second at power-up,
 *     N = demo number (1 original through 5 sweep), so the panel itself says
 *     which build is loaded.
 *
 * Everything that differs between the five demos is in demo_<variant>.cpp.
 */

#include "daisy_seed.h"
#include "alchemy/hw/alchemy_lab.h"
#include "alchemy/control/clock_follower.h"
#include "alchemy/control/musical_clock.h"
#include "alchemy/host_link/host.h"
#include "alchemy/surface/control_loop.h"
#include "alchemy/surface/jack.h"
#include "alchemy/surface/manual.h"
#include "alchemy/surface/page.h"
#include "alchemy/surface/pager.h"
#include "alchemy/surface/presets.h"
#include "alchemy/surface/settings.h"
#include "alchemy/surface/virtual_button.h"
#include "alchemy/surface/virtual_knob.h"

#include "lock_demo.h"

using namespace alchemy;

/* ── Clock input ───────────────────────────────────────────────────────── */

/* Pulses per quarter note expected on J2.  Must divide 96.  24 is the
 * SDK default (MIDI / DIN-sync rate).  Use 4 for a 16th-note clock or 1
 * for a quarter-note clock. */
static constexpr uint32_t kClockPpqn = kClockExtPpqnDefault;   /* 24 */

/* ── Pot colors: neutral, one per row ──────────────────────────────────── */

static constexpr LedPanel::Rgb kCyan   = {0x00, 0xC0, 0xFF};
static constexpr LedPanel::Rgb kOrange = {0xFF, 0x80, 0x00};
static constexpr LedPanel::Rgb kGreen  = {0x00, 0xFF, 0x40};

/* ── Knobs: plain 0..1 params with static Level arcs ──────────────────── */

static VirtualKnob p1 = VirtualKnob(kPotTopLeft,     "P1").Ident("p1").Ring(Level(kCyan));
static VirtualKnob p2 = VirtualKnob(kPotTopRight,    "P2").Ident("p2").Ring(Level(kCyan));
static VirtualKnob p3 = VirtualKnob(kPotMiddleLeft,  "P3").Ident("p3").Ring(Level(kOrange));
static VirtualKnob p4 = VirtualKnob(kPotMiddleRight, "P4").Ident("p4").Ring(Level(kOrange));
static VirtualKnob p5 = VirtualKnob(kPotBottomLeft,  "P5").Ident("p5").Ring(Level(kGreen));
static VirtualKnob p6 = VirtualKnob(kPotBottomRight, "P6").Ident("p6").Ring(Level(kGreen));

static Page main_page = Page(0).Name("Locks").Color("#00c0ff")
                            .Knobs(p1, p2, p3, p4, p5, p6);

/* Descriptor metadata for the web editor / manual. */
static Jack clock_in = Jack("J2", "Clock", JackSig::Trig);

static VirtualButton lock_button = VirtualButton("b1", "Lock")
    .Action("Hold + turn a pot",          "Record a lock on that pot")
    .Action("Hold + nudge a looping pot", "Clear its lock");

/* ── Surfaces ──────────────────────────────────────────────────────────── */

static AlchemyLab     hw;
static ControlLoop    loop    (hw);
static Pager          pager   (hw.buttons[kButtonB1], 1, kNumPots);  /* one page */
static Locks          locks   (hw.buttons[kButtonB1], pager);        /* B1 records */
static Presets        presets (hw.seed.qspi);
static Settings       settings(hw, &pager);
static MusicalClock   clock_  (kClockPpqnInternal, kClockBeatsPerBarDefault);
static ClockFollower  follower(clock_);

static hostlink::Host host(presets, kDemo.id, kDemo.name, "0.1.0", "demo");

static Manual manual = Manual()
    .Tagline(kDemo.tagline)
    .Preamble(kDemo.summary)
    .Section("how-to", "How to use",
             "Hold **B1** and turn a pot to record a lock on it; let go and "
             "the motion loops.  Hold B1 and nudge a looping pot to clear "
             "it.  Patch a clock into **J2** and choose **Clocked** on P5 "
             "in settings (hold B2 + B3) to snap loop lengths to the beat.  "
             "P6 chooses whether the knob **Returns** to where the gesture "
             "started or **Latches** where it ended.");

/* ── 1 ms poll: J2 edge → follower → NCO ────────────────────────────────── */

static void OnPoll(uint32_t /*t_ms*/)
{
    const uint32_t now_us = loop.PollTimeUs();
    if (hw.j2.RisingEdge()) follower.OnPulse(now_us);
    follower.Update(now_us);
    clock_.Tick(now_us);
}

/* ── Boot identifier: P1..PN white for a second, N = demo number ──────── */

static void ShowDemoNumber()
{
    const LedPanel::Rgb white = hw.leds.ScaleGlobal({0xFF, 0xFF, 0xFF});
    hw.leds.Clear();
    for (uint8_t ring = 0; ring < kDemo.number && ring < kNumPots; ring++)
        for (uint8_t led = 0; led < kLedsPerRing; led++)
            hw.leds.SetRingByOffset(ring, led, white);
    hw.leds.Show();
    daisy::System::Delay(1000);
}

/* ── Audio: silence.  Only here so the shim runs J2's edge detector. ──── */

static void AudioCallback(daisy::AudioHandle::InputBuffer  /*in*/,
                          daisy::AudioHandle::OutputBuffer out, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        out[0][i] = 0.f;
        out[1][i] = 0.f;
    }
}

int main()
{
    hw.Init();

    /* Clock on J2.  Freewheel keeps clocked loops running through a
     * dropout; the transport starts itself on the first lock. */
    follower.SetLossPolicy(ClockLossPolicy::Freewheel);
    follower.SetAutoStart(true);
    follower.Enable(kClockPpqn);

    /* SDK defaults, then whatever this variant changes. */
    locks.UseClock(clock_);
    kDemo.configure(locks);

    pager.SetPageColor(0, kCyan);   /* B1 tint */

    /* Default settings stack: P1 brightness, P3–P4 presets, P5–P6 locks.
     * UseLocks must follow UseClock. */
    settings.UseBrightness();
    settings.UsePresets(presets);
    settings.UseLocks(locks).DefaultExit(kDemo.exit_default);
    settings.Page(0).Help("Hold **B2 + B3** to enter settings.");

    main_page.Help(
        "Six normalized parameters shown as level arcs. Audio output is silent.");
    clock_in.Help("External clock.  Free/Clocked is P5 in settings.");
    lock_button
        .Help("The lock button.")
        .GestureHelp("Hold + turn a pot",
                     "Records the motion and loops it once B1 is released.")
        .GestureHelp("Hold + nudge a looping pot", "Clears that lock.");
    host.Jacks(clock_in).Buttons(lock_button).Attach(manual);

    presets.Manage(pager);
    presets.Manage(locks);
    presets.Manage(settings);
    presets.Init();
    presets.BootLoad();

    hw.StartAudio(AudioCallback);

    ShowDemoNumber();

    loop.Use(pager)
        .Use(locks)
        .Use(settings)
        .Use(host)
        .Use(main_page)
        .OnPoll(OnPoll);

    for (;;) loop.Tick();
}
