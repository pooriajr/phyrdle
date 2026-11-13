#!/usr/bin/env python3
"""Validate the explicit curated target list and generate its game files."""
import argparse
import csv
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
ANSWER_COUNT = 2000
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


def select_curated_answers(approved, guesses, blocked, count, *, excluded_plurals, frequencies):
    """Approval determines membership; frequency only orders the review report."""
    if count <= 0:
        raise ValueError("answer count must be positive")
    if len(approved) != len(set(approved)):
        raise ValueError("curated targets must be unique")
    if len(approved) != count:
        raise ValueError(f"expected exactly {count} curated targets, got {len(approved)}; "
                         "review replacements explicitly instead of automatic backfilling")
    if approved != sorted(approved):
        raise ValueError("curated targets must be alphabetical")
    for word in approved:
        if not WORD.fullmatch(word) or len(set(word)) != 5:
            raise ValueError(f"{word}: targets require five distinct lowercase letters")
        if word not in guesses:
            raise ValueError(f"{word}: curated target must also be in guesses.txt")
        if word in blocked:
            raise ValueError(f"{word}: target was explicitly excluded by curation or the blocklist")
        if word in excluded_plurals:
            raise ValueError(f"{word}: regular noun plurals are guesses only")
    return sorted(approved, key=lambda word: (-frequencies.get(word, 0), word))


def build_answers(check=False):
    source = ROOT / "sources"
    metadata = json.loads((source / "provenance.json").read_text())
    for filename, key in [("known_pool.txt", "known_pool_sha256"),
                          (metadata["frequency_subset_file"], "frequency_subset_sha256"),
                          ("plural_forms.tsv", "plural_subset_sha256")]:
        actual = hashlib.sha256((source / filename).read_bytes()).hexdigest()
        if actual != metadata[key]:
            raise ValueError(f"{filename}: source checksum does not match provenance.json")
    known = set((source / "known_pool.txt").read_text().splitlines())
    guesses = set((ROOT / "guesses.txt").read_text().splitlines())
    frequencies = {}
    for line in (source / metadata["frequency_subset_file"]).read_text().splitlines():
        word, count = line.split("\t")
        if not WORD.fullmatch(word) or word not in guesses or word in frequencies or int(count) <= 0:
            raise ValueError(f"invalid or duplicate frequency row: {line}")
        frequencies[word] = int(count)
    if len(known) != metadata["known_pool_rows"] or len(frequencies) != metadata["frequency_subset_rows"]:
        raise ValueError("source row counts do not match provenance.json")
    with (source / "plural_forms.tsv").open() as stream:
        reader = csv.DictReader(stream, delimiter="\t")
        if reader.fieldnames != ["word", "lemma", "pos", "source_line"]:
            raise ValueError("invalid plural source header")
        rows = list(reader)
    forms = []
    candidate_pool = guesses | known
    for row in rows:
        if (row["word"] not in candidate_pool or not row["lemma"]
                or row["pos"] not in {"ns", "nss"} or int(row["source_line"]) < 1):
            raise ValueError(f"invalid plural row: {row}")
        forms.append((row["word"], row["lemma"]))
    if (len(rows) != metadata["plural_subset_rows"]
            or len({word for word, _ in forms}) != metadata["plural_subset_words"]):
        raise ValueError("plural source counts do not match provenance.json")
    blocked = {line.split("#", 1)[0].strip()
               for line in (ROOT / "answer_blocklist.txt").read_text().splitlines()}
    with (ROOT / "curation_exclusions.tsv").open() as stream:
        blocked.update(row["word"] for row in csv.DictReader(stream, delimiter="\t"))
    approved = (ROOT / "curated_targets.txt").read_text().splitlines()
    ranked = select_curated_answers(
        approved, guesses, blocked, ANSWER_COUNT,
        excluded_plurals=regular_plural_exclusions(forms), frequencies=frequencies)
    outputs = {
        "answers.txt": "".join(word + "\n" for word in sorted(ranked)),
        "answer_ranking.tsv": "rank\tword\twikipedia_occurrences\n" + "".join(
            f"{rank}\t{word}\t{frequencies.get(word, 'NA')}\n"
            for rank, word in enumerate(ranked, 1)),
    }
    for filename, content in outputs.items():
        target = ROOT / filename
        if check:
            if not target.exists() or target.read_text() != content:
                raise ValueError(f"{filename} is stale; run make dictionary")
        else:
            target.write_text(content)
    print(f"Curated targets: {len(ranked):,}; "
          f"{sum(word not in frequencies for word in ranked)} without a Wikipedia count. "
          "No automatic backfill.")
    return ranked


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    try:
        build_answers(check=args.check)
    except ValueError as error:
        parser.exit(1, f"{error}\n")
