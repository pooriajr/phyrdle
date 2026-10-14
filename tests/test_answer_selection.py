import sys
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "words"))
from build_answers import rank_answers


class AnswerSelectionTest(unittest.TestCase):
    def test_filter_before_ranking_and_resolve_ties_alphabetically(self):
        frequencies = {"apple": 1000, "their": 900, "james": 800,
                       "ab1de": 700, "longer": 600, "crane": 10,
                       "flans": 10, "years": 5}
        known = {"apple", "their", "ab1de", "longer", "crane", "flans", "years"}
        selected, available = rank_answers(frequencies, known, {"their"}, 2)
        self.assertEqual(selected, ["crane", "flans"])
        self.assertEqual(available, 3)

    def test_insufficient_pool_fails_instead_of_silently_shortening(self):
        with self.assertRaisesRegex(ValueError, "only 1 eligible"):
            rank_answers({"crane": 10}, {"crane"}, set(), 2)
        with self.assertRaisesRegex(ValueError, "positive"):
            rank_answers({"crane": 10}, {"crane"}, set(), 0)


if __name__ == "__main__":
    unittest.main()
