/**
 * demo_new_default.cpp : ParamLock with the SDK v0.11.0 defaults.
 *
 *   - Arms at 0.5 % of travel, clears at 3 %.
 *   - Solid red pip while recording, blinking red pip while looping.
 */

#include "lock_demo.h"

using namespace alchemy;

static void Configure(Locks& /*locks*/)
{
    /* Nothing to change: this variant is the SDK's own defaults. */
}

constexpr LockDemoSpec kDemo = {
    2,
    LockExit::Return,
    "lock_demo_new_default",
    "Lock Demo 2: New Default",
    "v0.11.0 locks: 0.5 % arm threshold, solid red / blinking red pips",
    "Recording arms after 0.5% of pot travel. A solid red pip indicates "
    "recording; a blinking red pip indicates playback. The default exit "
    "mode is Return: releasing B1 restores the gesture's starting value "
    "and re-arms pot catch.",
    &Configure,
};
