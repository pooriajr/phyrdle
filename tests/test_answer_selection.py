import csv
import sys
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "words"))
from build_answers import select_curated_answers, regular_plural_exclusions


class AnswerSelectionTest(unittest.TestCase):
    def test_approval_controls_membership_instead_of_frequency_or_auto_backfill(self):
        approved = ["crane", "spork"]
        selected = select_curated_answers(
            approved, {"crane", "spork", "synod"}, set(), 2,
            excluded_plurals=set(), frequencies={"synod": 100000, "crane": 10})
        self.assertEqual(selected, ["crane", "spork"])
        with self.assertRaisesRegex(ValueError, "review replacements explicitly"):
            select_curated_answers(
                ["crane"], {"crane", "spork", "synod"}, set(), 2,
                excluded_plurals=set(), frequencies={"synod": 100000})

    def test_invalid_or_unreviewed_target_manifest_is_rejected(self):
        cases = [(["crane", "crane"], "unique"),
                 (["spork", "crane"], "alphabetical"),
                 (["apple"], "distinct"), (["CRANE"], "lowercase"),
                 (["crane"], "explicitly excluded"),
                 (["years"], "plurals"), (["spork"], "guesses")]
        for approved, message in cases:
            with self.subTest(approved=approved, message=message):
                with self.assertRaisesRegex(ValueError, message):
                    select_curated_answers(
                        approved, {"crane", "apple", "CRANE", "years"},
                        {"crane"} if message == "explicitly excluded" else set(),
                        len(approved), excluded_plurals={"years"}, frequencies={})

    def test_regular_plurals_are_excluded_but_irregular_plurals_can_be_approved(self):
        forms = [("years", "year"), ("cries", "cry"), ("boxes", "box"),
                 ("women", "woman"), ("fungi", "fungus"), ("media", "medium"),
                 ("jeans", "jean"), ("wives", "wife")]
        excluded = regular_plural_exclusions(forms)
        self.assertEqual(excluded, {"years", "cries", "boxes", "jeans"})
        approved = sorted(["fungi", "media", "wives", "women", "virus", "bonus", "focus"])
        selected = select_curated_answers(
            approved, set(approved), set(), len(approved),
            excluded_plurals=excluded, frequencies={})
        self.assertEqual(selected, approved)

    def test_irregular_sense_and_unchanged_plural_remain_eligible(self):
        self.assertEqual(regular_plural_exclusions([
            ("lives", "live"), ("lives", "life"), ("trout", "trout"),
            ("years", "year"), ("cries", "cry"), ("boxes", "box")]),
            {"years", "cries", "boxes"})

    def test_spelling_variants_keep_the_correct_singular_lemma(self):
        source = Path(__file__).resolve().parents[1] / "words/sources/plural_forms.tsv"
        with source.open() as stream:
            forms = [(row["word"], row["lemma"])
                     for row in csv.DictReader(stream, delimiter="\t")]
        excluded = regular_plural_exclusions(forms)
        for pair in [("grams", "gram"), ("grays", "gray"), ("jails", "jail"),
                     ("loves", "love"), ("molds", "mold")]:
            self.assertIn(pair, forms)
            self.assertIn(pair[0], excluded)


if __name__ == "__main__":
    unittest.main()
