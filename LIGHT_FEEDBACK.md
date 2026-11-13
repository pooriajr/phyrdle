# Letter and result lighting

The recognition acknowledgment and guess results use different behavior:

- Empty and occupied slots rest at the same dim neutral white.
- A newly recognized letter gives only its slot a 200 ms white pulse. The
  pulse rises and falls once, then returns to the neutral baseline. Replacing
  a letter gives another pulse; removing a tile or an unknown reading does not.
- A complete valid word leaves 200 ms for recognition, then reveals one result
  every 67 ms from left to right (about 50% faster than the original 100 ms
  steps). All non-winning valid words use this schedule, including all-absent
  guesses. The full sequence lasts 535 ms.
- Absent letters stay visibly lit gray-white (RGB 100, 100, 100), above the
  neutral baseline (40, 40, 40). Misplaced letters stay orange; correct letters
  stay green.
- Denied (invalid) words show two soft red pulses together on all five lights
  over one second: each pulse fades down for 250 ms and back up for 250 ms.
  Red intensity falls from 255 to 48, keeping a faint glow instead of turning
  completely off. The lights then hold steady red for two seconds. The
  three-second cycle repeats without abrupt brightness changes at its boundaries.
  Changing the invalid guess restarts the pulses. Removing a tile or forming a
  valid word stops the animation immediately, including during the steady hold.
- Winning guesses immediately start an animated rainbow at the leftmost slot,
  then extend it one slot every 67 ms. All five slots join within 268 ms; there
  is no recognition delay or all-green stage. Slots ahead of the sweep retain
  their quiet neutral brightness. The rainbow keeps its existing hue speed and
  the game resets eight seconds after the win is detected.
- Scanning continues throughout recognition, denial, and the non-winning result
  reveal. Any letter change cancels the current reveal; a new complete valid
  guess starts a fresh one.

Timing and relative white intensities are in `light_feedback.h`. RGB values are
before the existing FastLED global brightness of 50/255. Check these through
the actual enclosure in both bright and dim rooms: absent gray must be visibly
lit and distinct from the quiet neutral baseline. These values are a prototype
starting point, not a hardware-calibrated brightness specification.

Run `make test-light-feedback` for native timing and interruption checks, and
`make compile` to build the Nano firmware. A useful physical check is to place
an incomplete guess, complete an all-absent word, remove a tile during reveal,
replace one without emptying the whole board, try an invalid word and remove
a tile during both its pulsing and steady-red phases, and complete a winning guess.
