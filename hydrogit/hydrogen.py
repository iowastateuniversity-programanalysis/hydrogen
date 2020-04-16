from pathlib import Path

class HydrogenAdapter:
    def __init__(self, hydrogen_binary):
        self.hy=Path(hydrogen_binary)
        assert self.hy.exists()

    def call(self):
        pass
