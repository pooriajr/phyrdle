# Phyrdle dictionary

The game accepts **11,736 guesses** and picks its target from the **1,500 most
frequent eligible five-letter words in the supplied English Wikipedia snapshot
(April 13, 2023)**. Targets have five distinct letters for the physical tiles and exclude noun plurals.
The previous target pool is replaced; Wikipedia frequencies now determine
which words can be targets.

## How targets are selected

`build_answers.py` applies these rules before taking the top 1,500:

1. Exactly five lowercase letters, in the supplied SCOWL/ESDB size-60 US pool.
   This conservative dictionary filter reduces names, abbreviations, and raw
   Wikipedia token noise; it is not a manual review of every word.
2. Five distinct letters, so no target needs a duplicate physical tile.
3. Exclude entries in `answer_blocklist.txt`, copied from the supplied project.
   Blocked targets can still be guesses.
4. Exclude words labelled as noun plurals (`ns`/`nss`) in the matching
   ESDB revision. This catches regular forms such as `years` and `cries`,
   irregular forms such as `women`, `fungi`, and `media`, and plural-only
   words such as `jeans`. Singular words ending in S, including `virus`,
   `bonus`, and `focus`, remain eligible.
5. Sort by decreasing Wikipedia occurrence count, with alphabetical ties.

There are 2,391 eligible candidates after filtering. The 1,500th target is
`peony`, at 962 occurrences. The first ten are `first their after other would
later world while about under`. The last ten are `poise swank wiper flaky
hiker miser tromp anise bract peony`.

These are the top 1,500 **after filtering**, not the first 1,500 raw Wikipedia
tokens. Wikipedia frequency reflects encyclopedia writing, so some terms are
still specialized. Other inflections, including past-tense and verb-only
third-person forms, remain eligible; the requested restriction is noun
plurals. A frequency ranking does not guarantee a familiar puzzle answer.

Plural classification uses all noun entries in the pinned ESDB source,
including uncommon inflections, rather than a trailing-S heuristic. It is
conservative: a word listed as a noun plural is excluded even if it also has
a singular, verb, or other meaning (`corps`, `trout`, and `chaos`, for example).
The labelled source snapshot contains 1,620 records for 1,583 five-letter
words from the conservative pool. It excluded 402 of the previous 1,500
targets; the next eligible words backfilled the pool to 1,500. All of these
words remain accepted guesses.

## Reviewing and regenerating

- `answer_ranking.tsv`: all selected targets, in frequency order, with their
  rank and exact occurrence count. Use this to review the cutoff.
- `answers.txt`: the same selected targets alphabetically, for the encoder.
  This file is generated; do not edit it directly.
- `answer_blocklist.txt`: editable answer exclusions; selection backfills from
  the next eligible words when exclusions change.
- `guesses.txt`: the broad guess dictionary, including regional spellings,
  repeated-letter words, and obscure entries. It retains all 11,736 words
  accepted by the initial integration, even when they are no longer targets.
- `sources/known_pool.txt`: the supplied 5,200-word conservative dictionary.
- `sources/enwiki-2023-04-13-conservative.tsv`: every Wikipedia frequency row
  matching that dictionary, before the distinct-letter or blocklist filters.
  All 5,151 matching rows are included; there is no frequency cutoff here.
- `sources/plural_forms.tsv`: noun-plural labels with their lemma and line
  number in the matching ESDB source, for reviewing the exclusions.
- `sources/provenance.json`: source URLs, revisions, row counts, and SHA-256
  hashes for the original frequency/inflection sources and committed snapshots.

From the sketch directory:

```sh
make dictionary          # select targets, write ranking, then compress all data
make dictionary-check    # verify targets, ranking, source hashes, and encoded data
make test-dictionary     # selection-policy tests + exhaustive native tests
make compile             # complete Arduino Nano game at 8 MHz
```

`ANSWER_COUNT` in `build_answers.py` controls the target count (currently 1,500).
To exclude a target, edit the blocklist and run `make dictionary`. Commit
`answers.txt`, `answer_ranking.tsv`, and `src/wordset/wordset_data.h` together.
No downloads or sibling folders are needed to regenerate from the committed
snapshots. Ordinary Arduino IDE and upload-script builds use the committed
header and require neither Python nor a separate library installation.

The plural snapshot can be rebuilt from a local ESDB checkout at revision
`1e5b7d3a72f47a71da5d28686c1dd4b397178485`:

```sh
python3 words/import_plural_forms.py /path/to/wordlist
make dictionary
```

The importer verifies the full source hash, uses the matching ESDB parser,
and retains noun-plural entries matching the conservative dictionary. It
records source line numbers, updates the snapshot hash, and needs no database
build. Ordinary regeneration uses the committed snapshot and does not import
or download ESDB code.

The original `wordle_official_list.txt` remains historical source material;
it no longer controls targets. The former `word_list.h`, duplicate
`filtered_word_list.h`, and old filter script have been retired. Input
validation catches invalid target letters, duplicates, repeated target
letters, and data exceeding the decoder's index/bucket capacities.

## How compression works

Sorted guesses are grouped into 676 buckets by their first two letters.
A lookup jumps directly to the corresponding prefix bucket. The last three
letters become numbers from 0 to 17,575; the stream stores gaps between those
numbers, Rice-coded with a setting chosen for each bucket. A lookup decodes
just that bucket and stops once it finds or passes the sought suffix. The
largest bucket has 210 words.

An answer bitmap marks which guesses are targets, at one bit per **guess**.
Increasing or reducing the answer count changes those flags without changing
the bitmap's size or storing target spellings twice.

| Component | Flash bytes |
| --- | ---: |
| Compressed suffix stream | 13,976 |
| Bucket offsets | 1,352 |
| Bucket counts | 676 |
| Bucket compression settings | 338 |
| Target bitmap | 1,467 |
| **Total** | **17,809** |

Five-byte strings for these guesses alone would use 58,680 bytes. This
encoding uses 69.7% less space, including target selection. Membership is
exact, with no probabilistic false positives. It improves space efficiency;
it should not be described as universally faster than binary search. Board
timing has not been measured.

`dictionary.h` preserves the game's uppercase `String` API; WordSet works in
lowercase internally. Target draws continue to use the existing seeded RNG
and may repeat. The optional WordSet no-repeat picker is not used, and its
RAM tracker is removed by the linker. No additional EEPROM history is written.

## Source review and validation

The decoder and adapted encoder/test originated in the supplied WordSet
project, commit `d009fe3` (library metadata names Kris as author). Accepted
guesses come from SCOWL/ESDB, rather than raw Wikipedia tokens. All 11,713
supplied guesses matched the union of lowercase five-letter entries in its
US/British/Canadian/Australian size-95 exports. The integration retains four
additional words from the old source list and 19 former target words absent
from that supplied dictionary, for 11,736 total guesses.

The actual ESDB export headers identify revision `1e5b7d3`, June 24, 2026.
Some supplied README counts and fetch-script variant settings differ from
those actual files; committed snapshots are used here. The larger
`words_alpha.txt` was not blindly merged because it includes abbreviations
and name-like entries. The supplied word sets include offensive terms as
guesses; target exclusions are controlled separately by the blocklist.

The old validator accepted any five-character string ending in `S`, including
`ZZZZS`; this integration replaces that shortcut with dictionary membership.

Tests verify regular/irregular plural exclusion, preservation of singular
S-ending targets, continued plural guess acceptance, filter-before-ranking
behavior, alphabetical frequency ties,
insufficient candidate handling, current source hashes, and exact agreement
between generated targets and encoded data. The original 33 MB Wikipedia
file was also checked independently against the final ranked selection.
Native tests cover all guesses in both cases, malformed/nonsense inputs,
every target, index boundaries, 10,000 random target draws, and the library's
optional no-repeat API. They sweep all **11,881,376** five-letter strings to
prove exact agreement with the guess list. The host shim covers the Arduino
String/RNG interface used by the adapter; real AVR compilation is separate.

Nano builds use Arduino AVR core 1.8.8, FastLED 3.10.2, and an 8 MHz CPU:

| Build | Flash / 30,720 bytes | Static RAM / 2,048 bytes |
| --- | ---: | ---: |
| Original game | 21,356 | 1,094 |
| Wikipedia targets, default profile | 29,242 (95%) | 1,088 (53%) |

The default build leaves 1,478 bytes of flash and 960 bytes outside static RAM
allocations. Static RAM reporting does not include runtime stack/heap use.
No firmware was uploaded; physical-device responsiveness has not been tested.

## Attribution

`NOTICE.txt` preserves the supplied ESDB/UK Advanced Cryptics Dictionary
notices and adds the Wikipedia word-frequency project's MIT notice and
LDNOOBW attribution for the supplied answer blocklist. Source URLs and
snapshot hashes are in `sources/provenance.json`. The plural labels come
from the same ESDB revision as the dictionary exports and are covered by the
preserved ESDB data notice.

The supplied WordSet project has no standalone code LICENSE file. Its
metadata names its author but does not specify a code licence; a
redistribution licence for the copied WordSet code still needs to be
established before publishing it.
