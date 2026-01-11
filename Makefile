export CC = gcc
export CXX = g++ -std=c++20
export CFLAGS
export CXXFLAGS
export LDFLAGS
export AR = ar
export PKG_CONFIG = pkg-config

CFLAGS += -g -Os -ffunction-sections -fdata-sections
CXXFLAGS = $(CFLAGS) --std=c++17
LDFLAGS += -ffunction-sections -fdata-sections

export PREFIX = /usr/local

export REALOBJ = .obj
export OBJ = $(REALOBJ)/$(BUILDTYPE)

.PHONY: all
all: +all

clean::
	$(hide) rm -rf $(REALOBJ)

.PHONY: install
install: +all
	test -f bin/wordgrinder && cp bin/wordgrinder $(PREFIX)/bin/wordgrinder
	test -f bin/wordgrinder.1 && cp bin/wordgrinder.1 $(PREFIX)/man/man1/wordgrinder.1

.PHONY: debian-distr
debian-distr: bin/wordgrinder-minimal-dependencies-for-debian.tar.xz

.PHONY: bin/wordgrinder-minimal-dependencies-for-debian.tar.xz
bin/wordgrinder-minimal-dependencies-for-debian.tar.xz:
	@echo Make Debian distribution
	$(hide) mkdir -p $(dir $@)
	$(hide) tar cvaf $@ \
		--transform "s,^,wordgrinder-$(VERSION)/," \
		--exclude "*.dictionary" \
		Makefile \
		README \
		README.Windows.txt \
		README.wg \
		build.py \
		config.py \
		extras \
		licenses \
		scripts \
		src \
		testdocs \
		tests \
		third_party/luau \
		tools \
		wordgrinder.man

include build/ab.mk
