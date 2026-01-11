from build.ab import export
from config import (
    TEST_BINARY,
    VERSION,
)

export(
    name="all",
    items={
        "bin/wordgrinder$(EXT)": TEST_BINARY,
        "bin/wordgrinder.1": "extras+wordgrinder.1",
    },
    deps=["tests", "src/lua+typecheck"],
)
