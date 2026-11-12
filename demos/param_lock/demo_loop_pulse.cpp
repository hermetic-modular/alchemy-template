/**
 * demo_loop_pulse.cpp : new defaults, LoopPulse playback style.
 *
 *   - Thresholds and red color as v0.11.0.
 *   - The pip flashes at every loop restart and decays over the first
 *     fifth of the loop; solid while recording (there is no loop yet).
 */

#include "lock_demo.h"

using namespace alchemy;

static void Configure(Locks& locks)
{
    locks.RecordStyle(LockAnim::LoopPulse)
         .PlayStyle  (LockAnim::LoopPulse);
}

constexpr LockDemoSpec kDemo = {
    4,
    LockExit::Return,
    "lock_demo_loop_pulse",
    "Lock Demo 4: Loop Pulse",
    "new defaults, red pip, LoopPulse playback style",
    "Default thresholds with a red pip. During playback the pip "
    "flashes each time the loop restarts and fades over the first fifth "
    "of the loop, so you can see the loop length.  Solid while recording.",
    &Configure,
};
