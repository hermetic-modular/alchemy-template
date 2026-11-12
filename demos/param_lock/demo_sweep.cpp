/**
 * demo_sweep.cpp : new defaults, Sweep style.
 *
 *   - Thresholds and red color as v0.11.0.
 *   - While looping, the pip orbits the ring with the play head.  While
 *     recording it crawls with the write head, showing buffer fill
 *     (16 s is one full turn).
 */

#include "lock_demo.h"

using namespace alchemy;

static void Configure(Locks& locks)
{
    locks.RecordStyle(LockAnim::Sweep)
         .PlayStyle  (LockAnim::Sweep);
}

constexpr LockDemoSpec kDemo = {
    5,
    LockExit::Return,
    "lock_demo_sweep",
    "Lock Demo 5: Sweep",
    "new defaults, red pip, Sweep style",
    "Default thresholds with a red pip. During playback the pip travels "
    "around the ring with the play head, one lap per loop.  While "
    "recording it crawls from 6 o'clock with the write head, showing how "
    "much of the 16-second buffer is used.",
    &Configure,
};
