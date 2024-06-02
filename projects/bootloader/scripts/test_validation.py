def test_validate_board_id_valid(self):
        self.assertTrue(self.app.validate_board_id(1))

def test_validate_board_id_invalid(self):
    self.assertFalse(self.app.validate_board_id(-1))
    self.assertFalse(self.app.validate_board_id("invalid"))