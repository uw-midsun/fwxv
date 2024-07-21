import unittest
from validation import Validation


class TestValidation(unittest.TestCase):

    def test_validate_board_id_valid(self):
        self.assertTrue(Validation.validate_board_id(0))
        self.assertTrue(Validation.validate_board_id(15))
        self.assertFalse(Validation.validate_board_id(-1))
        self.assertFalse(Validation.validate_board_id(16))
        self.assertFalse(Validation.validate_board_id('invalid'))

    def test_validate_board_id_invalid(self):
        self.assertTrue(Validation.validate_jump_size(0))
        self.assertTrue(Validation.validate_jump_size(pow(2, 16) - 1))
        self.assertFalse(Validation.validate_jump_size(-1))
        self.assertFalse(Validation.validate_jump_size(pow(2, 16)))
        self.assertFalse(Validation.validate_jump_size('invalid'))


if __name__ == "__main__":
    unittest.main()
