# Phyrdle dictionary

The game accepts **11,739 guesses** and draws targets from **2,000 explicitly
curated words**. All targets have five distinct letters for the physical tiles.
Regular noun plurals are guesses only; familiar irregular plurals are allowed.

Start with [the full target list](curated_targets.txt) and
[the curation report](CURATION.md). Target membership is determined by that
approval list. Wikipedia frequency is only an optional review aid.

## Editing and regeneration

- `curated_targets.txt`: the authoritative, reviewed pool. Exactly 2,000 unique,
  alphabetically sorted lowercase words. Replace an entry with another reviewed
  word when editing the pool; the generator never automatically backfills.
- `curation_exclusions.tsv`: explicit editorial rejections, with reason
  categories. Rejected words remain eligible guesses.
- `answer_blocklist.txt`: the supplied answer-only exclusions, retained.
- `guesses.txt`: the broad dictionary, including repeated-letter words, regular
  plurals, obscure terms, and regional spellings. All 11,736 previously accepted
  guesses are preserved, with `spork`, `swole`, and `gouda` added.
- `answers.txt`: generated alphabetical copy of the approved pool, consumed by
  the encoder. Edit `curated_targets.txt` rather than this file.
- `answer_ranking.tsv`: the same approved words ordered by their Wikipedia
  occurrence counts. `NA` means no occurrence count was found in the snapshot.
  Frequency never adds a word to the approval pool.

From the sketch directory:

```sh
make dictionary          # validate approvals and regenerate all game word data
make dictionary-check    # reject stale outputs or invalid approvals/source hashes
make test-dictionary     # curation-policy checks and exhaustive native verification
make compile             # complete Arduino Nano game at 8 MHz
```

Commit approval changes, generated outputs, and the generated
`src/wordset/wordset_data.h` together. Add genuinely new words to `guesses.txt`
as well. Invalid spelling, repeated target letters, duplicate approvals,
regular noun plurals, rejected words, unknown guesses, and incorrect approval
counts stop the build. A stale header check also fails instead of silently
changing the approved pool.

Ordinary Arduino IDE and upload-script builds use the committed header. They
need neither Python nor a separately installed library. Regeneration needs no
network access or sibling folders.

`wordle_official_list.txt` remains historical source material and no longer
controls targets. The former packed headers and filter script were retired.

## Plurals and other forms

The pinned ESDB noun-inflection data identifies real noun plurals before any
spelling rule is applied. The regular forms `lemma+s`, `lemma+es`, and
consonant-`y` to `-ies` are excluded from targets. Other recorded plural forms
are treated as irregular; a recorded irregular reading keeps a word eligible
for editorial approval. Examples include `fungi`, `women`, `media`, `wives`,
and `lives`. Irregular forms are still reviewed for familiarity.

Source lemmas are matched by ESDB spelling/variant key, so `molds` is paired
with `mold` instead of the neighboring British `mould` spelling. The snapshot
covers noun forms matching the union of the conservative dictionary and the
broad guess set: 3,132 records for 3,086 words. This covers the wider curated
pool as well as the initial conservative one. Ordinary past tenses and
verb-only third-person forms remain permitted when explicitly approved.

The plural snapshot can be rebuilt from a local checkout of ESDB revision
`1e5b7d3a72f47a71da5d28686c1dd4b397178485`:

```sh
python3 words/import_plural_forms.py /path/to/wordlist
make dictionary
```

The importer checks the raw source hash and uses the matching ESDB parser.
Ordinary regeneration reads the committed snapshot and never downloads code.

## Sources and provenance

- `sources/known_pool.txt`: the supplied 5,200-word conservative SCOWL/ESDB
  size-60 US pool, retained for provenance and comparison.
- `sources/enwiki-2023-04-13-guesses.tsv`: all matching frequency rows from
  the supplied April 13, 2023 English Wikipedia file for the current guess
  dictionary. It expands the previous conservative-only snapshot so broader
  approved words can appear in the review report.
- `sources/plural_forms.tsv`: noun-plural labels, singular lemma, and source
  line number from the matching ESDB revision.
- `sources/provenance.json`: source URLs, revisions, row counts, hashes,
  curation method, and dictionary references for the three added guesses.

The supplied guesses matched the union of lowercase five-letter entries in
its US/British/Canadian/Australian size-95 exports. The imported decoder and
adapted encoder/test originated in supplied WordSet commit `d009fe3`, whose
library metadata names Kris as author. The actual ESDB export headers identify
revision `1e5b7d3`, June 24, 2026. Some supplied fetch settings differ from those
files; committed snapshots are used here.

The supplied `words_alpha.txt` was not blindly imported. The old shortcut
accepting any five-character string ending in S was removed; guesses now
require actual dictionary membership.

## Compression and device budget

Guesses are sorted into 676 buckets by their first two letters. The remaining
three letters become numbers from 0 to 17,575; the stream stores the gaps
between those numbers using Rice coding, with a setting chosen per bucket.
A lookup jumps to and decodes only the relevant prefix bucket, stopping once
it finds or passes the sought suffix. The largest bucket has 210 words.

An answer bitmap marks approved targets at one bit per **guess**, so no target
spelling is stored twice. Changing approvals does not resize the bitmap unless
the guess dictionary changes.

| Component | Flash bytes |
| --- | ---: |
| Compressed suffix stream | 13,979 |
| Bucket offsets | 1,352 |
| Bucket counts | 676 |
| Bucket compression settings | 338 |
| Target bitmap | 1,468 |
| **Total** | **17,813** |

Five-byte guess strings alone would occupy 58,695 bytes. The compressed data
uses 69.7% less space, including target selection. Membership is exact and has
no probabilistic false positives. Space efficiency improves; lookup speed on
physical hardware has not been measured.

The game keeps its uppercase String API. Target draws continue using the
existing seeded RNG and may repeat. The optional library no-repeat picker is
not used, and its tracker is removed by the linker. No repeat-history or
additional EEPROM behavior has been introduced.

## Verification

Selection tests check that explicit approvals control membership regardless of
frequency, gaps never auto-fill, invalid approvals fail, regular/irregular
plural policy is preserved, and spelling variants retain the right lemmas.
Native tests cover the game adapter, every guess/target, rejected-target
regressions, added words, case conversion, malformed input, index boundaries,
10,000 random target draws, and the optional library no-repeat API. They sweep
all **11,881,376** five-letter strings to prove exact dictionary membership.

The host shim covers only the Arduino String/RNG interface used by the adapter.
Real AVR builds are checked separately using Arduino AVR core 1.8.8, FastLED
3.10.2, and an 8 MHz CPU. See final build measurements below.

No firmware was uploaded; physical-device responsiveness has not been tested.
Static RAM reports exclude runtime stack/heap use.

## Attribution

`NOTICE.txt` preserves the supplied ESDB/UK Advanced Cryptics Dictionary
notices, Wikipedia word-frequency project's MIT notice, and LDNOOBW attribution
for the answer blocklist. The added dictionary references are in `CURATION.md`.

The supplied WordSet code has no standalone LICENSE file. Its metadata names
its author but gives no code licence; redistribution permission for that
copied code still needs to be established before publishing it.
