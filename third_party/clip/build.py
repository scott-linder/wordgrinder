from build.c import cxxlibrary
from build.pkg import package

cxxlibrary(
    name="clip_common",
    srcs=["./clip.cpp", "./image.cpp"],
    hdrs={
        "clip.h": "./clip.h",
        "clip_lock_impl.h": "./clip_lock_impl.h",
        "clip_common.h": "./clip_common.h",
    },
)

cxxlibrary(
    name="clip_none",
    srcs=["./clip_none.cpp"],
    hdrs={"clip.h": "./clip.h"},
    deps=[".+clip_common"],
)
