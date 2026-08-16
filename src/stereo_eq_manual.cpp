/**
 * The firmware-authored interactive manual.
 *
 * By defining these fields, you can embed your manual into the firmware
 * and enjoy using the programmer on hermeticmodular.com to interactively
 * view it. You only need to add prose.  Ranges, units, defaults, and 
 * labels are structured from the control declarations.
 *
 * Plug in via the front USB-C and view the module live on the programmer
 * and see where each manual string appears on the viewer!
 */

#include "alchemy/host_link/host.h"
#include "alchemy/surface/jack.h"
#include "alchemy/surface/manual.h"
#include "alchemy/surface/page.h"
#include "alchemy/surface/settings.h"
#include "alchemy/surface/virtual_button.h"
#include "alchemy/surface/virtual_knob.h"

using namespace alchemy;

/* You can put these in a separate header file, here for simplicity. */
extern VirtualKnob l_hi_level, l_hi_freq, l_mid_level, l_mid_freq,
                   l_lo_level, l_lo_freq;
extern VirtualKnob r_hi_level, r_hi_freq, r_mid_level, r_mid_freq,
                   r_lo_level, r_lo_freq;
extern Page          left_page, right_page;
extern Settings      settings;
extern Jack          in_l, in_r, cv1, cv2, cv3, cv4, cv5, cv6, out_l, out_r;
extern VirtualButton page_button;

static Manual manual =
    Manual()
        .Tagline("Dual mono three-band EQ")
        .Preamble(
            "**Stereo EQ is a dual mono three-band equalizer.** An "
            "independent high shelf, mid bell, and low shelf per channel, "
            "±24 dB per band.  The Left page shapes In L, the Right page "
            "shapes In R.")
        .Section("signal-flow", "Signal Flow",
                 "Each channel is one path: input → low shelf → mid bell → "
                 "high shelf → output.  The **Left** page's six pots are the "
                 "left channel; the **Right** page mirrors them for the "
                 "right.  CV 1–6 modulate the left channel's six parameters. "
                 "Remap them in the `CvMatrix` if you want CV elsewhere.")
        .Section("make-it-yours", "Make It Yours",
                 "This firmware is the template's worked example.  Gut the "
                 "DSP, rename the knobs, and rewrite this file — the manual "
                 "you are reading is `src/stereo_eq_manual.cpp`, and every "
                 "entry on this page is one `.Help()` call in it.");

void DescribeManual(hostlink::Host& host)
{
    left_page.Help(
        "The left channel's EQ.  Level pots cut or boost their band; Freq "
        "pots place it.");
    right_page.Help(
        "The right channel's EQ.");

    l_hi_level.Help("High-shelf gain. Everything above **Hi Freq.**")
        .SeeAlso(l_hi_freq, r_hi_level);
    l_hi_freq.Help("Corner of the high shelf.");
    l_mid_level.Help("Mid-bell gain.");
    l_mid_freq.Help("Center of the mid bell.");
    l_lo_level.Help(
        "Low-shelf gain.")
        .SeeAlso(l_lo_freq);
    l_lo_freq.Help("You can also selectively omit these notes.");

    r_hi_level.Help("As Hi Level on the Left page, for the right channel.")
        .SeeAlso(l_hi_level);
    r_hi_freq.Help("As Hi Freq, right channel.");
    r_mid_level.Help("As Mid Level, right channel.");
    r_mid_freq.Help("As Mid Freq, right channel.");
    r_lo_level.Help("As Lo Level, right channel.");
    r_lo_freq.Help("As Lo Freq, right channel.");

    in_l.Help("Left input. The Left page's channel.");
    in_r.Help("Right input. The Right page's channel.");
    cv1.Help("Summed with **Hi Level** on the left channel.")
        .SeeAlso(l_hi_level);
    cv2.Help("Summed with **Hi Freq** on the left channel.")
        .SeeAlso(l_hi_freq);
    cv3.Help("Summed with **Mid Level** on the left channel.")
        .SeeAlso(l_mid_level);
    cv4.Help("Summed with **Mid Freq** on the left channel.")
        .SeeAlso(l_mid_freq);
    cv5.Help("Summed with **Lo Level** on the left channel.")
        .SeeAlso(l_lo_level);
    cv6.Help("Summed with **Lo Freq** on the left channel.")
        .SeeAlso(l_lo_freq);
    out_l.Help("Left output, post-EQ.");
    out_r.Help("Right output, post-EQ.");

    page_button
        .Help("Page switch and motion-loop recorder in one button.")
        .GestureHelp("Tap",
                     "Switches between the Left and Right pages.  Pots "
                     "catch on switch: turn through a value to pick it up.")
        .GestureHelp("Hold + turn a pot",
                     "Records that pot's movement as a loop and replays it "
                     "until recorded again.  Loops are saved with presets.");

    settings.Page(0).Help(
        "Hold **B2 + B3** to enter settings; B1 leaves.");

    host.Jacks(in_l, in_r, cv1, cv2, cv3, cv4, cv5, cv6, out_l, out_r)
        .Buttons(page_button)
        .Attach(manual);
}
