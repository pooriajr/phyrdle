# The 2,000 curated targets

The complete, authoritative list is [curated_targets.txt](curated_targets.txt).
It contains exactly 2,000 unique, alphabetically sorted words. Every word has
five distinct letters and is an accepted guess.

This is an explicit assistant familiarity review for a general adult
English-speaking audience. It is not a population recognition survey, and it
does not claim that a measured percentage of people knows every word.

## Review standard

The intended reaction to a difficult answer is recognition: a player can see
why it was a fair word to solve. The pool includes everyday vocabulary and
recognizable written English, so targets can be challenging without requiring
a specialist dictionary.

The review considered the full distinct-letter candidate pool from both the
conservative and broader supplied dictionaries. A first pass selected familiar
words and rejected narrow technical terms, archaic words, opaque regionalisms,
fragments, and unnatural or uncommon forms. A second pass reconsidered
borderline choices and added familiar replacements. A final pass removed
additional weak targets, including `cloys` and the mechanical verb `trued`.

Compared with the previous 1,000-word Wikipedia-ranked pool, the final list
keeps 960 words, removes 40, and adds 1,040 reviewed words. Of the final pool,
1,919 words occur in the original conservative dictionary; 81 come from the
broader guess dictionary or the three separately verified additions below.

The record [curation_exclusions.tsv](curation_exclusions.tsv) contains 563
explicit exclusions and their editorial reason categories. Most unselected
words from the larger guess dictionary remain guesses without needing an
individual exclusion record. An approval is always required to become a target.

## Choices retained deliberately

- Ordinary past tenses and other natural inflections are allowed: `baked`,
  `cawed`, `pored`, `spied`, `woken`, and similar forms remain fair words.
- Regular noun plurals such as `years`, `boxes`, and `cries` are guesses only.
  Familiar irregular plurals such as `fungi`, `women`, `wives`, `lives`, and
  `media` are approved. Irregular status alone does not guarantee approval:
  the unfamiliar `septa`, `phyla`, and `carpi` are excluded.
- Common British and American spellings can both be targets. Examples include
  `fiber`/`fibre`, `liter`/`litre`, and `miter`/`mitre`. The pool also includes
  familiar forms such as `leant`, `spelt`, and `spilt`.
- Familiar loanwords, foods, materials, and lexicalized labels are allowed:
  `adieu`, `anime`, `gouda`, `mochi`, `queso`, `lycra`, and `mylar`, for example.
  These are selected for their ordinary vocabulary uses, not as people or
  place-name answers.
- Recognizable contemporary language is represented by words such as `admin`,
  `emoji`, `janky`, `judgy`, `snark`, `swole`, `techy`, `unbox`, and `untag`.
- Standard adult written vocabulary such as `demur`, `exult`, `posit`, and
  `thrum` remains eligible as a reasonable challenge. Familiarity varies with
  readers' ages, regions, and interests; these choices are editorial judgments.

The previous problem entries `begum`, `burgh`, `canto`, `copra`, `ketch`,
`laird`, `lough`, `quint`, `sager`, `sahib`, `septa`, `synod`, `talus`,
`taxon`, and `rives` are excluded from targets and remain legal guesses.

## Extra dictionary verification

Three familiar approved words were missing from the supplied guess set, so
both guess coverage and the target pool now include them. These are independent
editorial choices, not words automatically selected from dictionary definitions:

- [spork — Merriam-Webster](https://www.merriam-webster.com/dictionary/spork)
- [swole — Merriam-Webster](https://www.merriam-webster.com/dictionary/swole)
- [Gouda, the cheese noun — Merriam-Webster](https://www.merriam-webster.com/dictionary/Gouda)

The guess dictionary now contains 11,739 words. No previously accepted guess
was removed. The definitions themselves are not copied into this repository.

## Keeping curation stable

`build_answers.py` requires exactly 2,000 explicit approvals. It validates
spelling, tile compatibility, guess membership, regular-plural status, and
exclusion conflicts. It never fills a gap from Wikipedia or an unreviewed list.
To change the pool, explicitly remove one approval, review a replacement, add
it to the approval file (and guesses if necessary), and regenerate.

Wikipedia occurrence counts remain available in `answer_ranking.tsv` as an
optional review view. They do not determine target membership. `NA` means the
word was not found in the supplied frequency snapshot; it does not mean the
word is invalid or has zero familiarity. `cawed`, `clomp`, and `rewax` are the
three approved words without a recorded count.
