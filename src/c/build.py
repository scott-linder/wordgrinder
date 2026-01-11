from build.ab import simplerule
from build.c import cxxprogram, cxxlibrary
from build.pkg import package
from build.zip import zip
from build.utils import itemsof
from os.path import *
from glob import glob
from config import (
    FILEFORMAT,
    DEFAULT_DICTIONARY_PATH,
)

package(name="libcmark", package="libcmark", fallback="third_party/cmark")
package(name="fmt", package="fmt", fallback="third_party/fmt")

cxxlibrary(
    name="globals",
    srcs=[
        "./utils.cc",
        "./cmark.cc",
        "./filesystem.cc",
        "./main.cc",
        "./screen.cc",
        "./word.cc",
        "./zip.cc",
    ],
    hdrs={"globals.h": "./globals.h", "script_table.h": "src/lua+luacode"},
    cflags=[
        f"-DFILEFORMAT={FILEFORMAT}",
        "-DCMARK_STATIC_DEFINE",
        "-I.",
    ],
    caller_cflags=[f"-DFILEFORMAT={FILEFORMAT}"],
    deps=[
        ".+fmt",
        ".+libcmark",
        ".+libncursesw",
        "third_party/luau",
        "third_party/wcwidth",
        "third_party/minizip",
        "third_party/libstb",
    ],
)


def make_wordgrinder(name, deps=[], cflags=[], ldflags=[]):
    cxxprogram(
        name=name,
        srcs=[
            "./lua.cc",
            "./clipboard.cc",
        ],
        cflags=cflags
        + [
            f"-DDEFAULT_DICTIONARY_PATH={DEFAULT_DICTIONARY_PATH}",
        ],
        ldflags=ldflags,
        deps=[
            ".+libcmark",
            ".+globals",
            "third_party/clip+clip_common",
            "third_party/luau",
            "third_party/minizip",
            "third_party/wcwidth",
        ]
        + deps,
    )

package(name="libncursesw", package="ncursesw")

make_wordgrinder(
    "wordgrinder",
    deps=[
        "third_party/clip+clip_none",
        "third_party/wcwidth",
    ],
)
