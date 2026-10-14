# Phyrdle dictionary

The game accepts **11,736 dictionary guesses** while choosing targets only
from the **2,138 existing curated words**, all with five distinct letters.
`dictionary.h` preserves the game's uppercase `String` API; the compressed
WordSet library works in lowercase internally.

## Editing the lists

- `answers.txt`: the complete curated target pool, one lowercase word per line.
  Keep it sorted, unique, and free of repeated letters for the physical tiles.
- `guesses.txt`: the broad guess dictionary. Repeated letters are allowed here.
  It contains the supplied 11,713-word dictionary plus `amaro`, `masse`,
  `unsee`, and `yeesh` from the existing `wordle_official_list.txt`.
- The generator automatically adds all answers to the accepted guesses. That
  preserves 19 local targets absent from the supplied dictionary:
  `capri crema flowy glamp glowy hacky hinky incel janky judgy koran mochi nutso
  petri queso quran tased untag vitro`.

From the sketch directory:

```sh
make dictionary          # regenerate src/wordset/wordset_data.h
make dictionary-check    # fail if committed data differs from the lists
make test-dictionary     # regeneration check + exhaustive native tests
make compile             # complete Arduino Nano game at 8 MHz
```

Commit the edited lists and generated header together. Ordinary Arduino IDE
and upload-script builds use the committed header and need no Python or
separate library installation. `wordle_official_list.txt` remains historical
source material; edit `words/answers.txt` to change targets now. The former
`word_list.h`, duplicate `filtered_word_list.h`, and filter script were retired
so there is one authoritative target list.

The generator rejects malformed, duplicate, unsorted, or repeated-letter
targets and rejects data that exceeds the decoder's index/bucket capacities.

## What is stored

Words are sorted into 676 buckets by their first two letters (`aa` to `zz`).
A lookup jumps directly to that prefix's bucket. The remaining three letters
are represented by numbers from 0 to 17,575. Instead of storing every number,
the dictionary stores the gaps between successive numbers using Rice coding,
with a compression setting selected separately for each bucket.

Only that bucket is decoded, stopping as soon as the sought suffix is found
or passed. The largest bucket has 210 words. Membership is exact: it does not
use a probabilistic filter and never accepts a word absent from the generated
list. Dictionary membership still reflects the chosen source's vocabulary;
it is not a guarantee of every word's suitability for every English game.

A bitmap marks which guess words are targets, at one bit per **guess**, so
answers do not store their spelling twice. The complete data occupies:

| Component | Flash bytes |
| --- | ---: |
| Compressed suffix stream | 13,976 |
| Bucket offsets | 1,352 |
| Bucket counts | 676 |
| Bucket compression settings | 338 |
| Target bitmap | 1,467 |
| **Total** | **17,809** |

Five-byte strings for these guesses alone would use 58,680 bytes, larger
than the Nano's flash. This encoding reduces that by 69.7%, including target
selection. It improves space efficiency; it trades binary search for a short
sequential decode, so it should not be described as universally faster.
Board timing has not been measured.

Target draws continue to use the RNG seeded by the existing setup code and
may repeat. WordSet also provides an optional no-repeat picker, but the game
does not call it; its RAM tracker is removed by the linker. This integration
does not write additional EEPROM history.

## Review of the supplied folder

The imported decoder and adapted encoder/test originated in `wikipedia words`,
WordSet commit `d009fe3` (library metadata names Kris as author). The complete
original folder is left intact alongside this repository.

The folder's name is slightly misleading: **accepted guesses come from
SCOWL/ESDB dictionary exports**. Wikipedia token frequencies rank the friend's
alternative 1,000-answer pool. That automatic answer pool is not used here:
289 of its words repeat letters, and frequency alone is not game curation.

Checks on the actual files:

- All 11,713 supplied guesses are unique, sorted, lowercase five-letter words.
  They match the union of lowercase five-letter entries in the included
  US/British/Canadian/Australian size-95 exports exactly.
- The original 1,000 answers are a subset of both its guesses and its
  conservative dictionary pool. The provided decoder passed its original
  exhaustive test, answer enumeration, and no-repeat-pool tests.
- The download headers identify ESDB revision `1e5b7d3`, June 24, 2026, with
  variant level 8 for the size-95 US file. The fetch script's requested variant
  parameter and some README counts differ from those actual files. This
  integration uses committed snapshots, with no downloads in regeneration.
- The extra `data/words_alpha.txt` contains 15,921 five-letter entries, including
  obvious abbreviations and name-like entries such as `accts`, `addnl`, and
  `aaron`. It was not blindly merged. Raw Wikipedia tokens are not used as
  valid guesses either.
- The supplied set accepts regional spellings and obscure words and includes
  offensive terms as guesses. It is broad, but no finite list covers every
  valid English word. Add reviewed missing words to `guesses.txt`.
- All 2,138 original target spellings were retained. The previous header
  incorrectly declared 2,137 entries and placed `SYNTH` before `SYNOD`, breaking
  the sort assumption and leaving the final word outside the declared range.
  Generated counts and ordering correct those defects.
- The old validator accepted every five-character string ending in `S`, even
  `ZZZZS`. That shortcut is removed: real plurals must be dictionary entries.

## Validation and device budget

`make test-dictionary` tests the real game adapter for all accepted words in
both cases, mixed-case input, malformed/nonsense input, every curated target,
index bounds, and 10,000 random target draws. It also tests the low-level
library's full enumeration, answer membership, and no-repeat API and sweeps
all **11,881,376** five-letter strings to prove exact agreement with the input
lists. The host shim covers only the Arduino String/RNG interface used by the
adapter; real-device compilation is checked separately.

Nano builds (Arduino AVR core 1.8.8, FastLED 3.10.2, 8 MHz):

| Build | Flash / 30,720 bytes | Static RAM / 2,048 bytes |
| --- | ---: | ---: |
| Original game | 21,356 | 1,094 |
| Expanded dictionary, default profile | 29,242 (95%) | 1,088 (53%) |
| Expanded dictionary, mean8 calibrated profile | 29,260 (95%) | 1,108 (54%) |

The default-profile build leaves 1,478 bytes of flash and 960 bytes outside static
RAM allocations. Static RAM reporting does not include runtime stack/heap use.
No firmware was uploaded and physical-device responsiveness was not tested.

## Attribution

`NOTICE.txt` preserves the complete notice from the supplied size-95 US export,
including the ESDB permission and UK Advanced Cryptics Dictionary notice.
Carry this with distributed dictionary data. The local target list's existing
provenance is unchanged. Wikipedia ranking data and LDNOOBW's blocklist are
not imported or used by this build.

The supplied WordSet project has no standalone code LICENSE file; its library
metadata identifies its author but does not specify a code licence. This
branch records that provenance; a redistribution licence for the copied
WordSet code still needs to be established before publishing it.
