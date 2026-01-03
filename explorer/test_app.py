import unittest
from unittest.mock import Mock, patch

import explorer.app as explorer_app


class RpcCallTests(unittest.TestCase):
    @patch("explorer.app.requests.post")
    def test_rpc_call_returns_result(self, mock_post):
        response = Mock()
        response.raise_for_status.return_value = None
        response.json.return_value = {"result": {"height": 42}}
        mock_post.return_value = response

        result = explorer_app.rpc_call("getblockcount")

        self.assertEqual({"height": 42}, result)
        mock_post.assert_called_once()

    @patch("explorer.app.requests.post")
    def test_rpc_call_raises_on_error(self, mock_post):
        response = Mock()
        response.raise_for_status.return_value = None
        response.json.return_value = {"error": {"message": "boom"}}
        mock_post.return_value = response

        with self.assertRaises(RuntimeError):
            explorer_app.rpc_call("getblock", ["bad"])


if __name__ == "__main__":
    unittest.main()
