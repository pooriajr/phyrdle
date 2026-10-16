import sys
import csv
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "words"))
from build_answers import rank_answers, regular_plural_exclusions


class AnswerSelectionTest(unittest.TestCase):
    def test_filter_before_ranking_and_resolve_ties_alphabetically(self):
        frequencies = {"apple": 1000, "their": 900, "james": 800,
                       "ab1de": 700, "longer": 600, "crane": 10,
                       "flans": 10, "years": 5}
        known = {"apple", "their", "ab1de", "longer", "crane", "flans", "years"}
        selected, available = rank_answers(frequencies, known, {"their"}, 2, excluded_plurals=set())
        self.assertEqual(selected, ["crane", "flans"])
        self.assertEqual(available, 3)

    def test_insufficient_pool_fails_instead_of_silently_shortening(self):
        with self.assertRaisesRegex(ValueError, "only 1 eligible"):
            rank_answers({"crane": 10}, {"crane"}, set(), 2, excluded_plurals=set())
        with self.assertRaisesRegex(ValueError, "positive"):
            rank_answers({"crane": 10}, {"crane"}, set(), 0, excluded_plurals=set())

    def test_regular_plurals_are_excluded_but_irregular_plurals_can_rank(self):
        forms = [("years", "year"), ("cries", "cry"), ("boxes", "box"),
                 ("women", "woman"), ("fungi", "fungus"), ("media", "medium"),
                 ("jeans", "jean"), ("wives", "wife")]
        frequencies = {word: 100 for word, _ in forms}
        frequencies.update({"virus": 30, "bonus": 20, "focus": 10})
        selected, available = rank_answers(
            frequencies, set(frequencies), set(), 7,
            excluded_plurals=regular_plural_exclusions(forms))
        self.assertEqual(selected, ["fungi", "media", "wives", "women", "virus", "bonus", "focus"])
        self.assertEqual(available, 7)

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
        # The import must match ESDB spelling keys: e.g. molds belongs to
        # mold, not the neighboring British variant mould in the same group.
        for pair in [("grams", "gram"), ("grays", "gray"), ("jails", "jail"),
                     ("loves", "love"), ("molds", "mold")]:
            self.assertIn(pair, forms)
            self.assertIn(pair[0], excluded)


if __name__ == "__main__":
    unittest.main()
