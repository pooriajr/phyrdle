#!/usr/bin/env python3
"""Select target words using the committed Wikipedia-frequency snapshot."""
import argparse
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
ANSWER_COUNT = 1000
WORD = re.compile(r"[a-z]{5}")


def is_regular_plural(word, lemma):
    """Recognize the ordinary -s, -es, and consonant-y -> -ies patterns."""
    return (word in {lemma + "s", lemma + "es"}
            or (len(lemma) > 1 and lemma.endswith("y")
                and lemma[-2] not in "aeiou" and word == lemma[:-1] + "ies"))


def regular_plural_exclusions(forms):
    """Keep a word eligible if any recorded noun sense has an irregular plural."""
    regular, irregular = set(), set()
    for word, lemma in forms:
        (regular if is_regular_plural(word, lemma) else irregular).add(word)
    return regular - irregular


def rank_answers(frequencies, known_words, blocked, count, *, excluded_plurals):
    """Filter first, then rank by descending frequency with alphabetical ties."""
    if count <= 0:
        raise ValueError("answer count must be positive")
    eligible = [word for word in frequencies
                if WORD.fullmatch(word) and len(set(word)) == 5
                and word in known_words and word not in blocked
                and word not in excluded_plurals
                and frequencies[word] > 0]
    ranked = sorted(eligible, key=lambda word: (-frequencies[word], word))
    if len(ranked) < count:
        raise ValueError(f"only {len(ranked)} eligible words; cannot select {count}")
    return ranked[:count], len(ranked)


def build_answers(check=False):
    source = ROOT / "sources"
    metadata = json.loads((source / "provenance.json").read_text())
    # Catch truncated or accidentally edited source snapshots, rather than
    # silently changing which words are considered most frequent.
    for filename, key in [("known_pool.txt", "known_pool_sha256"),
                          ("enwiki-2023-04-13-conservative.tsv", "frequency_subset_sha256"),
                          ("plural_forms.tsv", "plural_subset_sha256")]:
        actual = hashlib.sha256((source / filename).read_bytes()).hexdigest()
        if actual != metadata[key]:
            raise ValueError(f"{filename}: source checksum does not match provenance.json")
    known = set((source / "known_pool.txt").read_text().splitlines())
    frequencies = {}
    for line in (source / "enwiki-2023-04-13-conservative.tsv").read_text().splitlines():
        word, count = line.split("\t")
        if not WORD.fullmatch(word) or word not in known or word in frequencies:
            raise ValueError(f"invalid or duplicate frequency row: {line}")
        frequencies[word] = int(count)
    if len(known) != metadata["known_pool_rows"] or len(frequencies) != metadata["frequency_subset_rows"]:
        raise ValueError("source row counts do not match provenance.json")
    plural_rows = (source / "plural_forms.tsv").read_text().splitlines()
    if plural_rows[0] != "word\tlemma\tpos\tsource_line":
        raise ValueError("invalid plural source header")
    plurals = set()
    plural_forms = []
    for row in plural_rows[1:]:
        word, lemma, pos, source_line = row.split("\t")
        if word not in known or not lemma or pos not in {"ns", "nss"} or int(source_line) < 1:
            raise ValueError(f"invalid plural row: {row}")
        plurals.add(word)
        plural_forms.append((word, lemma))
    if (len(plural_rows) - 1 != metadata["plural_subset_rows"]
            or len(plurals) != metadata["plural_subset_words"]):
        raise ValueError("plural source counts do not match provenance.json")
    blocked = {line.split("#", 1)[0].strip()
               for line in (ROOT / "answer_blocklist.txt").read_text().splitlines()}
    ranked, available = rank_answers(
        frequencies, known, blocked, ANSWER_COUNT,
        excluded_plurals=regular_plural_exclusions(plural_forms))
    outputs = {
        "answers.txt": "".join(word + "\n" for word in sorted(ranked)),
        "answer_ranking.tsv": "rank\tword\twikipedia_occurrences\n" + "".join(
            f"{rank}\t{word}\t{frequencies[word]}\n"
            for rank, word in enumerate(ranked, 1)),
    }
    for filename, content in outputs.items():
        target = ROOT / filename
        if check:
            if not target.exists() or target.read_text() != content:
                raise ValueError(f"{filename} is stale; run make dictionary")
        else:
            target.write_text(content)
    print(f"Wikipedia targets: {len(ranked):,} of {available:,} eligible; "
          f"cutoff {frequencies[ranked[-1]]:,} occurrences ({ranked[-1]})")
    return ranked


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    try:
        build_answers(check=args.check)
    except ValueError as error:
        parser.exit(1, f"{error}\n")
