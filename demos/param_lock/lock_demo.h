/**
 * lock_demo.h : what each demo variant supplies to the shared firmware.
 *
 * lock_demo.cpp is identical across the five demos; a variant is one
 * demo_<name>.cpp that defines `kDemo`: the HostLink identity, a line of
 * manual prose, and a function that applies its ParamLock thresholds and
 * pip styles on top of the SDK defaults.
 */

#pragma once

#include "alchemy/hw/alchemy_lab_layout.h"   /* kNumPots */
#include "alchemy/surface/param_lock.h"

/** One lock slot per pot on a single page; default length (16 s @ 30 Hz). */
using Locks = alchemy::ParamLock<alchemy::kNumPots>;

struct LockDemoSpec
{
    uint8_t     number;          ///< 1..5; shown as lit rings for a second at boot
    alchemy::LockExit exit_default;  ///< P6 at boot: Latch (v0.10.0) or Return
    const char* id;              ///< HostLink module id, e.g. "lock_demo_sweep"
    const char* name;            ///< display name, e.g. "Lock Demo 5: Sweep"
    const char* tagline;         ///< one line under the name in the manual
    const char* summary;         ///< manual preamble (markdown)
    void (*configure)(Locks&);   ///< this variant's thresholds + LED styles
};

/** Defined once, in demo_<variant>.cpp. */
extern const LockDemoSpec kDemo;
