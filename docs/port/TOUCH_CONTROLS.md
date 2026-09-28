# Touch controls

The gesture table is in the [README](../../README.md#touch-controls). This page is the rest: how
gestures are recognised, what is deliberately off on touch, and how to report a control problem.

Every gesture is classified only once the intent is unambiguous (a short
delay or distance threshold), so an ordinary tap never misfires into a
selection box, and pan and zoom don't flicker into each other.

Screen-edge scrolling is **off** on touch. It is defined by a pointer resting
near an edge, and it ends only when a later pointer event reports a position
back inside the safe zone — a condition that cannot occur without a pointer,
which is why it used to leave the camera scrolling on its own. Dragging with
a finger is the touch equivalent and is already direct.

## Player settings (launcher → Settings → Touch controls)

These change which existing gesture a finger becomes, never what that gesture orders, so they
are client-side only: multiplayer and replays are unaffected. They reach the engine as startup
arguments (`-gxLongPressMs`, `-gxDoubleTapDrag`, `-gxSmartTap`, `-gxCancelButton`,
`-gxCancelButtonSize`) and live in `Common/GXTouchSettings.h`.

- **Long press time** (0.3–2.0 s, default 0.6 s): how long a still finger on the map must rest for
  its release to cancel. Also governs the long-press escape from ability targeting. The minimap
  look, the shell right-click and a held UI button keep their own fixed 0.6 s.
- **Double-tap then drag** (on): tap, then touch the same spot again within the double-tap window
  (350 ms, 40 px) and drag -- a selection box immediately, without the hold that the
  press-hold-drag box needs.
- **Smart tap delay** (on, needs double-tap-drag): a tap that would *order* something
  (`TouchInput::tapIssuesOrder`, which walks `tap()`'s decisions with no side effects) is held for
  the double-tap window. A second touch that drags drops it -- the player was starting a box, not
  ordering; anything else sends it first, so the two still happen in the order they were tapped.
  Taps that only select stay instant.
- **Cancel button** (on, size small/medium/large): a ✕ drawn over everything by
  `InGameUI::postWindowDraw`, visible only in a match with the command bar up. A tap is the same
  cancel as the long press; a drag of more than 24 px moves it, and the release saves the
  position to `TouchButtons.ini` in the user-data folder, as a fraction of the screen so it
  survives a change of Screen Shape. **Reset position** in the launcher deletes that file.

If controls misbehave, turn on **Touch input overlay** in Setup →
Diagnostics: it draws the current gesture, where your finger is, where the
engine thinks the pointer is, and the camera's scroll anchor, and writes
matching `[gxtouch]` lines into the log. Those four things are the same thing
on a desktop and different things on a touchscreen, and their disagreement is
what every control bug here has turned out to be.

For how this is built and how to add a gesture, see
[`docs/WORKDIR/lessons/LESSON-touch-input-is-not-a-mouse.md`](../WORKDIR/lessons/LESSON-touch-input-is-not-a-mouse.md).
