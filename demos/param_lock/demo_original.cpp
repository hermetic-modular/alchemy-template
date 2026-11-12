/**
 * demo_original.cpp : ParamLock as it shipped in SDK v0.10.0.
 *
 *   - One 3 % gesture threshold: a pot had to travel 3 % before recording
 *     armed (v0.11 arms at 0.5 %), and the same 3 % cleared a lock.
 *   - Solid red pip while recording, solid green pip while looping.
 *   - Latch exit: on release the knob stays where the gesture ended and
 *     playback rides on top.  Return did not exist yet.
 */

#include "lock_demo.h"

using namespace alchemy;

static void Configure(Locks& locks)
{
    locks.ArmThreshold(0.03f)
         .ClearThreshold(0.03f)
         .RecordStyle({0xFF, 0x00, 0x00}, LockAnim::Solid)
         .PlayStyle  ({0x00, 0xFF, 0x40}, LockAnim::Solid);
}

constexpr LockDemoSpec kDemo = {
    1,
    LockExit::Latch,
    "lock_demo_original",
    "Lock Demo 1: Original",
    "v0.10.0 locks: 3 % arm, Latch exit, solid red / solid green pips",
    "Recording arms after 3% of pot travel. A solid red pip indicates "
    "recording; a solid green pip indicates playback. The default exit "
    "mode is Latch: releasing B1 keeps the pot at the gesture's final value.",
    &Configure,
};
