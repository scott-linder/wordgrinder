import subprocess
import os
import platform
from datetime import date

FILEFORMAT = 8
VERSION = "0.9"
DATE = date.today().strftime("%-d %B %Y")

TEST_BINARY = "src/c/+wordgrinder"

DEFAULT_DICTIONARY_PATH = "/usr/share/dict/words"
