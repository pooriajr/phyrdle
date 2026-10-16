#!/usr/bin/env python3
"""Rebuild the plural snapshot from a local checkout of the pinned ESDB revision."""
import argparse
import hashlib
import importlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("esdb_root", type=Path, help="ESDB checkout containing data/ and libscowl/")
    args = parser.parse_args()
    metadata_path = ROOT / "sources/provenance.json"
    metadata = json.loads(metadata_path.read_text())
    source = args.esdb_root.resolve() / "data/scowl-pre.txt"
    if hashlib.sha256(source.read_bytes()).hexdigest() != metadata["plural_source_sha256"]:
        parser.error("ESDB source does not match the pinned source hash")
    sys.path.insert(0, str(args.esdb_root.resolve()))
    core = importlib.import_module("libscowl._core")
    known = set((ROOT / "sources/known_pool.txt").read_text().splitlines())
    records = set()
    # Some groups put their plural row before the lemma; collect the entire
    # group first so every snapshot row has an explicit lemma for review.
    groups = source.read_text().split("\n\n")
    line_start = 1
    for group in groups:
        lines = group.split("\n")
        noun_lines = []
        lemmas_by_spelling = {}
        for offset, line in enumerate(lines):
            if "<n" not in line:
                continue
            match = core._matchLine(line)
            if match and match["base_pos"] == "n":
                part = core.parseLemmaPart(match["lemma"])
                spellings = core.Spellings.parse(match["spellings"] or "")
                key = (spellings.key(), match["num"])
                noun_lines.append((line_start + offset, match, part, spellings, key))
                if part.lemma:
                    if key in lemmas_by_spelling and lemmas_by_spelling[key] != part.lemma:
                        raise ValueError(f"conflicting lemmas at line {line_start + offset}")
                    lemmas_by_spelling[key] = part.lemma
        for number, match, part, spellings, key in noun_lines:
            lemma = lemmas_by_spelling.get(key)
            words = {}
            entry = core.WordEntry(part.lemma) if part.lemma else None
            core.Line.procWords(spellings.keys() if spellings else "_", entry, "n", match["words"], words)
            for pos in ("ns", "nss"):
                for form in words.get(pos, []):
                    if form.word in known:
                        if not lemma:
                            raise ValueError(f"missing lemma at line {number}")
                        records.add((form.word, lemma, pos, number))
        line_start += len(lines) + 1
    content = "word\tlemma\tpos\tsource_line\n" + "".join(
        f"{word}\t{lemma}\t{pos}\t{number}\n" for word, lemma, pos, number in sorted(records))
    target = ROOT / "sources/plural_forms.tsv"
    target.write_text(content)
    metadata["plural_subset_sha256"] = hashlib.sha256(target.read_bytes()).hexdigest()
    metadata["plural_subset_rows"] = len(records)
    metadata["plural_subset_words"] = len({record[0] for record in records})
    metadata_path.write_text(json.dumps(metadata, indent=2) + "\n")
    print(f"wrote {len(records)} plural records for {metadata['plural_subset_words']} words")


if __name__ == "__main__":
    main()
