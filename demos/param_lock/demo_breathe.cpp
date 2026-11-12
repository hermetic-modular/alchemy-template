/**
 * demo_breathe.cpp : new defaults, with a violet pip that breathes.
 *
 *   - Thresholds as v0.11.0 (0.5 % arm, 3 % clear).
 *   - Solid violet pip while recording, breathing violet pip while looping.
 */

#include "lock_demo.h"

using namespace alchemy;

static constexpr LedPanel::Rgb kViolet = {0xA0, 0x00, 0xFF};

static void Configure(Locks& locks)
{
    locks.RecordStyle(kViolet, LockAnim::Solid)
         .PlayStyle  (kViolet, LockAnim::Breathe);
}

constexpr LockDemoSpec kDemo = {
    3,
    LockExit::Return,
    "lock_demo_breathe",
    "Lock Demo 3: Breathe",
    "new defaults, violet pip, Breathe playback style",
    "Default thresholds with a violet pip. The pip is solid while recording "
    "and breathes on a two-second cycle while the "
    "slot loops.",
    &Configure,
};
