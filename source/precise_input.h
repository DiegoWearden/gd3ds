#pragma once

#include <3ds.h>

// Sub-frame (CBF-style) input timing.
//
// The HID sysmodule samples buttons/touch every ~4ms into an 8-entry ring in
// shared memory; libctru's hidScanInput() only reads the newest entry once per
// rendered frame, discarding the rest. This module reads the whole ring each
// frame, reconstructs timestamped press/release events, and replays them into
// state.input at the exact 240Hz physics step they occurred in, making input
// timing precision independent of the render framerate.

typedef enum {
    // Queue events for per-step replay.
    PRECISE_POLL_NORMAL,
    // Drop events but track the real held state (death/fade frames, where
    // physics is skipped or decoupled from real time).
    PRECISE_POLL_DISCARD,
    // Drop events and force held state to false (pause frames, mirroring the
    // kHeldPaused masking: a button held across unpause must not jump until
    // re-pressed).
    PRECISE_POLL_PAUSED,
} PreciseInputPollMode;

// Per-level CBF toggle (pause menu); when false, the game uses the vanilla
// once-per-frame input snapshot.
extern bool cbf_enabled;

// Call once when (re)entering the gameplay loop.
void precise_input_reset(void);

// Call once per frame, after hidScanInput().
void precise_input_poll(PreciseInputPollMode mode);

// True when events are flowing and the timeline is valid this frame.
bool precise_input_ready(void);

// Apply the input state for the physics step whose real-time window ends at
// window_end_tick (system ticks). Sets state.input and state.old_input.
void precise_input_step(u64 window_end_tick);

