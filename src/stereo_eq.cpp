/**
 * stereo_eq.cpp — Alchemy Lab dual mono three-band EQ.
 * Demonstrates a large surface of the SDK.
 *
 * Pure DSP in *_dsp files implements:
 * - Three-band EQ per channel, ±24 dB per band
 * - Independent left/right channels
 *
 * This file implements:
 * - All the required hardware and audio callback management.
 * - Two pages of controls, switched by B1
 * - Pot-catch on page switch (no value jumps)
 * - Record looping automation by holding B1 and moving a pot
 * - Each pot modulated by its assigned CV input
 * - Animations for each LED ring.
 * - LED ring per pot displaying summed value.
 * - Save and recall presets with flash wear leveling.
 * - Settings menu for managing basics
 * - HostLink: the /program web editor and `make program-live`, one line each
 * - Interactive manual: help on every control (see stereo_eq_manual.cpp)
 */

#include "daisy_seed.h"
#include "alchemy/hw/alchemy_lab.h"
#include "alchemy/host_link/host.h"
#include "alchemy/surface/control_loop.h"
#include "alchemy/surface/cv_matrix.h"
#include "alchemy/surface/jack.h"
#include "alchemy/surface/page.h"
#include "alchemy/surface/pager.h"
#include "alchemy/surface/param_lock.h"
#include "alchemy/surface/presets.h"
#include "alchemy/surface/settings.h"
#include "alchemy/surface/virtual_button.h"
#include "alchemy/surface/virtual_knob.h"

#include "stereo_eq_dsp.h"
#include "stereo_eq_palette.h"

/* see stereo_eq_manual.cpp. */
void DescribeManual(alchemy::hostlink::Host& host);

using namespace alchemy;

/* We define each knob's curve and LED Ring animation.  CV routing lives
 * in the CvMatrix declaration below; declaring it once at the matrix
 * level keeps the knob declarations purely about the knob.  */
static constexpr float kGainMaxDb = 24.f;

/* Page 1 */

/* .Ident() pins each field's id, .Unit() is for hostlink display. */
VirtualKnob l_hi_level = VirtualKnob(kPotTopLeft, "Hi Level")
    .Linear(-kGainMaxDb, +kGainMaxDb).Ident("l.hi.level").Unit("dB")
    .Ring(Bipolar(kLeftPalette.hi.level_pos,
                  kLeftPalette.hi.level_neg,
                  kLeftPalette.hi.level_center));

VirtualKnob l_hi_freq = VirtualKnob(kPotTopRight, "Hi Freq")
    .Exp(1000.f, 16000.f).Ident("l.hi.freq").Unit("Hz")
    .Ring(Level(kLeftPalette.hi.freq, FillAnim::Pulse));

VirtualKnob l_mid_level = VirtualKnob(kPotMiddleLeft, "Mid Level")
    .Linear(-kGainMaxDb, +kGainMaxDb).Ident("l.mid.level").Unit("dB")
    .Ring(Bipolar(kLeftPalette.mid.level_pos,
                  kLeftPalette.mid.level_neg,
                  kLeftPalette.mid.level_center));

VirtualKnob l_mid_freq = VirtualKnob(kPotMiddleRight, "Mid Freq")
    .Exp(200.f, 5000.f).Ident("l.mid.freq").Unit("Hz")
    .Ring(Level(kLeftPalette.mid.freq, FillAnim::Ripple));

VirtualKnob l_lo_level = VirtualKnob(kPotBottomLeft, "Lo Level")
    .Linear(-kGainMaxDb, +kGainMaxDb).Ident("l.lo.level").Unit("dB")
    .Ring(Bipolar(kLeftPalette.lo.level_pos,
                  kLeftPalette.lo.level_neg,
                  kLeftPalette.lo.level_center));

VirtualKnob l_lo_freq = VirtualKnob(kPotBottomRight, "Lo Freq")
    .Exp(60.f, 600.f).Ident("l.lo.freq").Unit("Hz")
    .Ring(Level(kLeftPalette.lo.freq, FillAnim::Pulse));


/* Page 2 */
VirtualKnob r_hi_level = VirtualKnob(kPotTopLeft, "Hi Level")
    .Linear(-kGainMaxDb, +kGainMaxDb).Ident("r.hi.level").Unit("dB")
    .Ring(Bipolar(kRightPalette.hi.level_pos,
                  kRightPalette.hi.level_neg,
                  kRightPalette.hi.level_center));

VirtualKnob r_hi_freq = VirtualKnob(kPotTopRight, "Hi Freq")
    .Exp(1000.f, 16000.f).Ident("r.hi.freq").Unit("Hz")
    .Ring(Level(kRightPalette.hi.freq, FillAnim::Pulse));

VirtualKnob r_mid_level = VirtualKnob(kPotMiddleLeft, "Mid Level")
    .Linear(-kGainMaxDb, +kGainMaxDb).Ident("r.mid.level").Unit("dB")
    .Ring(Bipolar(kRightPalette.mid.level_pos,
                  kRightPalette.mid.level_neg,
                  kRightPalette.mid.level_center));

VirtualKnob r_mid_freq = VirtualKnob(kPotMiddleRight, "Mid Freq")
    .Exp(200.f, 5000.f).Ident("r.mid.freq").Unit("Hz")
    .Ring(Level(kRightPalette.mid.freq, FillAnim::Ripple));

VirtualKnob r_lo_level = VirtualKnob(kPotBottomLeft, "Lo Level")
    .Linear(-kGainMaxDb, +kGainMaxDb).Ident("r.lo.level").Unit("dB")
    .Ring(Bipolar(kRightPalette.lo.level_pos,
                  kRightPalette.lo.level_neg,
                  kRightPalette.lo.level_center));

VirtualKnob r_lo_freq = VirtualKnob(kPotBottomRight, "Lo Freq")
    .Exp(60.f, 600.f).Ident("r.lo.freq").Unit("Hz")
    .Ring(Level(kRightPalette.lo.freq, FillAnim::Pulse));

/* Bind knobs to pages.  Name/Color label the web editor's tabs. */
Page left_page  = Page(0).Name("Left").Color("#00c0ff")
                      .Knobs(l_hi_level, l_hi_freq,
                             l_mid_level, l_mid_freq,
                             l_lo_level, l_lo_freq);
Page right_page = Page(1).Name("Right").Color("#ff8040")
                      .Knobs(r_hi_level, r_hi_freq,
                             r_mid_level, r_mid_freq,
                             r_lo_level, r_lo_freq);

/* Jack metadata: name + signal class per panel jack.  Pure descriptor
 * data. Routing in CvMatrix below. */
Jack in_l ("IN_L",  "In L",  JackSig::AudioIn);
Jack in_r ("IN_R",  "In R",  JackSig::AudioIn);
Jack cv1  ("J3",    "CV 1",  JackSig::CvBi);
Jack cv2  ("J4",    "CV 2",  JackSig::CvBi);
Jack cv3  ("J5",    "CV 3",  JackSig::CvBi);
Jack cv4  ("J6",    "CV 4",  JackSig::CvBi);
Jack cv5  ("J7",    "CV 5",  JackSig::CvBi);
Jack cv6  ("J8",    "CV 6",  JackSig::CvBi);
Jack out_l("OUT_L", "Out L", JackSig::AudioOut);
Jack out_r("OUT_R", "Out R", JackSig::AudioOut);

/* Button metadata: what B1's gestures do, for the editor's button chips. */
VirtualButton page_button = VirtualButton("b1", "Page / Lock")
    .Action("Tap", "Next page")
    .Action("Hold + turn a pot", "Record a motion loop");

/* Get our SDK surfaces and opt in to everything */
static AlchemyLab                        hw;
static ControlLoop                       loop    (hw);
static Pager                             pager   (hw.buttons[0], 2, kNumPots);
static ParamLock<2 * kNumPots>           locks   (hw.buttons[0], pager);
static Presets                           presets (hw.seed.qspi);
Settings                                 settings(hw, &pager);
static CvMatrix                          cv_matrix(kNumCvInputs);

/* HostLink: module identity once; transport, buffers, and reboot handling
 * are SDK defaults.  This is what the /program web editor talks to — and
 * what lets `make program-live` reboot the module for flashing. */
static hostlink::Host                    host    (presets, "stereo_eq",
                                                  "Stereo EQ", "0.1.0",
                                                  "template");

/* summed CV+knob values → DSP each frame */
static constexpr float kMidQ      = 1.2f;
static void UpdateCoeffs()
{
    eq_dsp::SetChannel(0, {
        l_lo_freq.Value(),  l_lo_level.Value(),
        l_mid_freq.Value(), l_mid_level.Value(), kMidQ,
        l_hi_freq.Value(),  l_hi_level.Value(),
    });
    eq_dsp::SetChannel(1, {
        r_lo_freq.Value(),  r_lo_level.Value(),
        r_mid_freq.Value(), r_mid_level.Value(), kMidQ,
        r_hi_freq.Value(),  r_hi_level.Value(),
    });
}

int main()
{
    hw.Init();
    eq_dsp::Init(hw.SampleRate());

    /* CV routing.  A static layout is just setting each channel once. */
    cv_matrix.Jack(0).To(l_hi_level);
    cv_matrix.Jack(1).To(l_hi_freq);
    cv_matrix.Jack(2).To(l_mid_level);
    cv_matrix.Jack(3).To(l_mid_freq);
    cv_matrix.Jack(4).To(l_lo_level);
    cv_matrix.Jack(5).To(l_lo_freq);

    /* Opting into default settings gestures and controls.*/
    settings.UseBrightness();
    settings.UsePresets(presets);

    /* See stereo_eq_manual.cpp. */
    DescribeManual(host);

    /* Preset payload — every Serializable surface gets walked on Save/Load. */
    presets.Manage(pager);
    presets.Manage(locks);
    presets.Manage(settings);
    presets.Init();
    presets.BootLoad();

    UpdateCoeffs();
    hw.StartAudio(eq_dsp::Process);

    /* ControlLoop is a thin, opt-in driver for the canonical control-rate frame.
     * If desired, you can unroll and modify. */
    loop.Use(pager)
        .Use(locks)
        .Use(settings)
        .Use(cv_matrix)
        .Use(host)
        .Use(left_page)
        .Use(right_page)
        .OnFrame(UpdateCoeffs);

    for (;;) loop.Tick();
}
