#include "precise_input.h"
#include "state.h"
#include "main.h"
#include "menus/settings.h"

// HID shared memory layout (words), per libctru hid.c / 3dbrew:
// PAD section at word 0:
//   [0..1] tick of latest pass (updated when entry 0 is written)
//   [2..3] tick of previous pass
//   [4]    index of newest entry (0..7)
//   [10 + i*4 + 0] entry i: held mask
//   [10 + i*4 + 3] entry i: circle pad (s16 dx, s16 dy)
// Touch section at word 42, same header layout:
//   [50 + i*2 + 0] entry i: u16 px | u16 py << 16
//   [50 + i*2 + 1] entry i: nonzero when touching
#define PAD_BASE 0
#define TOUCH_BASE 42
#define PAD_ENTRIES (PAD_BASE + 10)
#define TOUCH_ENTRIES (TOUCH_BASE + 8)
#define RING_SIZE 8
// libctru's threshold for synthesizing KEY_CPAD_* from circle pad coords
#define CPAD_THRESHOLD 41

bool cbf_enabled = true;

typedef struct {
    u64 tick;
    u8 is_touch;
    u8 held;
} InputEvent;

#define EVENT_QUEUE_SIZE 64

static InputEvent event_queue[EVENT_QUEUE_SIZE];
static int queue_head, queue_len;

// Ring cursor: identity of the last consumed sample per section
static u64 pad_pass_tick, touch_pass_tick;
static int pad_last_idx, touch_last_idx;

// Raw held state as of the last drained sample, per source
static bool btn_raw, touch_raw;
// Pause masking (kHeldPaused semantics): a source held when the pause ended
// stays suppressed until a genuine release clears the mask
static bool btn_masked, touch_masked;
// Effective held state (raw minus mask) as of the last drained sample
static bool btn_held, touch_held;
// Held state as of the last consumed event, per source
static bool step_btn_held, step_touch_held;
static KeyInput last_applied;

static bool ready;

static u64 read_tick(vu32 *mem)
{
    // 64-bit reads aren't atomic against the HID module; re-read until stable
    u64 t = ((u64)mem[1] << 32) | mem[0];
    for (int i = 0; i < 3; i++) {
        u64 t2 = ((u64)mem[1] << 32) | mem[0];
        if (t2 == t) break;
        t = t2;
    }
    return t;
}

static bool jump_btn_from_sample(u32 held, s16 cpad_y)
{
    u32 mask = (yJump ? KEY_Y : KEY_A) | KEY_DUP;
    return (held & mask) || cpad_y >= CPAD_THRESHOLD;
}

static bool jump_touch_from_sample(u32 xy, u32 touching)
{
    if (!touching) return false;
    u16 px = xy & 0xFFFF;
    u16 py = xy >> 16;
    // Same UI exclusion zones as the vanilla in_bounds path
    return !gameplay_touch_blocked(px, py);
}

static void queue_event(u64 tick, bool is_touch, bool held)
{
    if (queue_len == EVENT_QUEUE_SIZE) {
        // Overflow: drop the oldest event; precision degrades but state stays consistent
        queue_head = (queue_head + 1) % EVENT_QUEUE_SIZE;
        queue_len--;
    }
    InputEvent *ev = &event_queue[(queue_head + queue_len) % EVENT_QUEUE_SIZE];
    ev->tick = tick;
    ev->is_touch = is_touch;
    ev->held = held;
    queue_len++;
}

// Walk one section's ring, oldest to newest, invoking cb for unseen samples.
// Returns the newest sample tick via *pass_tick/*last_idx cursors.
typedef void (*sample_cb)(u64 tick, int idx);

static void drain_ring(int base, u64 *pass_tick, int *last_idx, sample_cb cb)
{
    // Snapshot header + index consistently: if the module starts a new pass
    // (writes entry 0) between these reads, a sample would be stamped one
    // pass (~34ms) off, so retry until the header is stable across the reads
    u64 cur, prev;
    int newest;
    for (int try = 0;; try++) {
        cur = read_tick(&hidSharedMem[base]);
        prev = read_tick(&hidSharedMem[base + 2]);
        newest = hidSharedMem[base + 4];
        if (read_tick(&hidSharedMem[base]) == cur || try == 3) break;
    }
    if (newest < 0 || newest > 7) return;
    if (cur == 0 || cur <= prev) return;

    u64 interval = (cur - prev) / RING_SIZE;

    // Previous pass: entries newest+1..7 (timestamps relative to prev),
    // then current pass: entries 0..newest (relative to cur)
    for (int n = 0; n < RING_SIZE; n++) {
        int i;
        u64 base_tick;
        if (n < RING_SIZE - 1 - newest) {
            i = newest + 1 + n;
            base_tick = prev;
        } else {
            i = n - (RING_SIZE - 1 - newest);
            base_tick = cur;
        }
        // Skip samples already consumed on a previous frame
        if (base_tick < *pass_tick) continue;
        if (base_tick == *pass_tick && i <= *last_idx) continue;
        cb(base_tick + (u64)i * interval, i);
    }
    *pass_tick = cur;
    *last_idx = newest;
}

static PreciseInputPollMode poll_mode;

static void pad_sample(u64 tick, int idx)
{
    vu32 *e = &hidSharedMem[PAD_ENTRIES + idx * 4];
    u32 held = e[0];
    s16 cpad_y = (s16)(e[3] >> 16);
    btn_raw = jump_btn_from_sample(held, cpad_y);
    if (!btn_raw) btn_masked = false;
    bool eff = btn_raw && !btn_masked;
    if (eff == btn_held) return;
    btn_held = eff;
    if (poll_mode == PRECISE_POLL_NORMAL) queue_event(tick, false, eff);
}

static void touch_sample(u64 tick, int idx)
{
    vu32 *e = &hidSharedMem[TOUCH_ENTRIES + idx * 2];
    touch_raw = jump_touch_from_sample(e[0], e[1]);
    if (!touch_raw) touch_masked = false;
    bool eff = touch_raw && !touch_masked;
    if (eff == touch_held) return;
    touch_held = eff;
    if (poll_mode == PRECISE_POLL_NORMAL) queue_event(tick, true, eff);
}

void precise_input_reset(void)
{
    queue_head = queue_len = 0;
    pad_pass_tick = touch_pass_tick = 0;
    pad_last_idx = touch_last_idx = RING_SIZE - 1;
    btn_raw = touch_raw = false;
    btn_masked = touch_masked = false;
    btn_held = touch_held = false;
    step_btn_held = step_touch_held = false;
    last_applied = state.input;
    ready = false;

    // Consume the whole ring so stale menu inputs never replay into gameplay;
    // held state settles to the real current state (holding at level start
    // jumps, like the vanilla kHeld path)
    poll_mode = PRECISE_POLL_DISCARD;
    drain_ring(PAD_BASE, &pad_pass_tick, &pad_last_idx, pad_sample);
    drain_ring(TOUCH_BASE, &touch_pass_tick, &touch_last_idx, touch_sample);
}

void precise_input_poll(PreciseInputPollMode mode)
{
    poll_mode = mode;

    if (mode == PRECISE_POLL_NORMAL && !ready) {
        // First live frame after a reset/pause/death: seed the per-step held
        // state from before this frame's samples, so edges queued below still
        // register as presses. After PAUSED frames this is false, so a button
        // held across unpause stays masked until re-pressed (kHeldPaused
        // semantics); after DISCARD frames it is the real held state, so a
        // jump buffered through the death animation still fires at respawn.
        step_btn_held = btn_held;
        step_touch_held = touch_held;
        last_applied = state.input;
    }

    drain_ring(PAD_BASE, &pad_pass_tick, &pad_last_idx, pad_sample);
    drain_ring(TOUCH_BASE, &touch_pass_tick, &touch_last_idx, touch_sample);

    if (mode == PRECISE_POLL_NORMAL) {
        ready = true;
    } else {
        queue_head = queue_len = 0;
        if (mode == PRECISE_POLL_PAUSED) {
            // Anything still held when the pause ends must not act until
            // released and pressed again (kHeldPaused semantics)
            btn_masked = btn_raw;
            touch_masked = touch_raw;
            btn_held = touch_held = false;
        }
        ready = false;
    }
}

bool precise_input_ready(void)
{
    return ready;
}

void precise_input_step(u64 window_end_tick)
{
    bool pressed = false;

    while (queue_len > 0) {
        InputEvent *ev = &event_queue[queue_head];
        if (ev->tick > window_end_tick) break;

        bool source_was = ev->is_touch ? step_touch_held : step_btn_held;
        if (ev->is_touch) step_touch_held = ev->held;
        else step_btn_held = ev->held;

        // Rising edge on either source counts as a press, matching how kDown
        // fires per-button in the vanilla path
        if (ev->held && !source_was) pressed = true;

        queue_head = (queue_head + 1) % EVENT_QUEUE_SIZE;
        queue_len--;
    }

    KeyInput in;
    in.pressedJump = pressed;
    in.holdJump = pressed || step_btn_held || step_touch_held;

    state.old_input = last_applied;
    state.input = in;
    last_applied = in;
}

