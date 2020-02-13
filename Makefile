# Compiler
CC:=gcc
WINCC:=x86_64-w64-mingw32-$(CC)
CXX:=g++
WINCXX:=x86_64-w64-mingw32-$(CXX)

# Flags
CSTD:=-std=gnu11
CXXSTD:=-std=gnu++17
WFLAGS:=-Wall -Wextra
OFLAGS:=-O3 -flto -fwhole-program
IFLAGS:=-I "include/"
CFLAGS:=$(CSTD) $(WFLAGS) $(OFLAGS) $(IFLAGS)
CXXFLAGS:=$(CXXSTD) $(WFLAGS) $(OFLAGS) $(IFLAGS)
LIBFLAGS:=-lm

# Query
QUERY_LEECH=leechgem/leechquery
QUERY_MANAGEM=managem/mgquery-alone managem/mgquery-amps managem/mgquery-setup managem/mgquery-omnia
QUERY_CRIT=critgem/critquery critgem/critquery-amps
QUERY_KILLGEM=killgem/kgquery-alone killgem/kgquery-amps killgem/kgquery-setup killgem/kgquery-omnia
QUERY_MGNGEM=managem/mgquery-ngems
QUERY_KGNGEM=killgem/kgquery-ngems
QUERY_BLEED=bleedgem/bleedquery
QUERY_DIST=$(QUERY_LEECH) $(QUERY_MANAGEM) $(QUERY_CRIT) $(QUERY_KILLGEM) $(QUERY_BLEED)
QUERY_ALL=$(QUERY_DIST) $(QUERY_MGNGEM) $(QUERY_KGNGEM)

# Build
BUILD_LEECH=leechgem/leechbuild
BUILD_MANAGEM=managem/mgbuild-appr managem/mgbuild-exact managem/mgbuild-c6
BUILD_CRIT=critgem/critbuild
BUILD_KILLGEM=killgem/kgbuild-appr killgem/kgbuild-exact killgem/kgbuild-c6
BUILD_BLEED=bleedgem/bleedbuild
BUILD_ALL=$(BUILD_LEECH) $(BUILD_MANAGEM) $(BUILD_CRIT) $(BUILD_KILLGEM) $(BUILD_BLEED)

# Combine
COMBINE_LEECH=leechgem/leechcombine
COMBINE_MANAGEM=managem/mgcombine-appr
COMBINE_CRIT=critgem/critcombine
COMBINE_KILLGEM=killgem/kgcombine-appr killgem/kgcombine-exact
COMBINE_BLEED=bleedgem/bleedcombine
COMBINE_ALL=$(COMBINE_LEECH) $(COMBINE_MANAGEM) $(COMBINE_CRIT) $(COMBINE_KILLGEM) $(COMBINE_BLEED)

# Type aggregates
LEECH_ALL=$(QUERY_LEECH) $(BUILD_LEECH) $(COMBINE_LEECH)
MANAGEM_ALL=$(QUERY_MANAGEM) $(QUERY_MGNGEM) $(BUILD_MANAGEM) $(COMBINE_MANAGEM)
CRIT_ALL=$(QUERY_CRIT) $(BUILD_CRIT) $(COMBINE_CRIT)
KILLGEM_ALL=$(QUERY_KILLGEM) $(QUERY_KGNGEM) $(BUILD_KILLGEM) $(COMBINE_KILLGEM)
BLEED_ALL=$(QUERY_BLEED) $(BUILD_BLEED) $(COMBINE_BLEED)

# Parser
PARSER=parser

# All
DEV_ALL=$(QUERY_ALL) $(BUILD_ALL) $(COMBINE_ALL) $(PARSER)

# Folders
BINDIR:=bin
WINDIR:=$(BINDIR)/win
TABLES_DIR:=gem_tables
INCLUDE_DIR:=include
DEPS_DIR:=.deps

# Version
VERSION:=$(shell git describe --tags | rev | cut -d '-' -f2- | rev)

# General targets
dev: $(DEV_ALL)
dev-move: dev
	$(MAKE) move

all: dev tables
all-move: all
	$(MAKE) move

dist: $(QUERY_DIST) tables
	$(MAKE) move
	@echo "Setup completed, the programs can be found in the \"$(BINDIR)/\" folder"

analyze:
	scan-build -V -analyze-headers $(MAKE)

.PHONY: dev dev-move all all-move dist analyze

# Aggregates
set-query: $(QUERY_ALL)
set-build: $(BUILD_ALL)
set-combine: $(COMBINE_ALL)

set-leech: $(LEECH_ALL)
set-managem: $(MANAGEM_ALL)
set-crit: $(CRIT_ALL)
set-killgem: $(KILLGEM_ALL)
set-bleed: $(BLEED_ALL)

.PHONY: set-query set-build set-combine set-leech set-managem set-crit set-killgem

# Windows
windows: CXX:=$(WINCXX)
windows: $(QUERY_DIST) windows-tables | $(WINDIR)
	$(foreach file, $(QUERY_DIST), mv -v $(file) $(WINDIR)/$(shell basename $(file)).exe;)
	cp LICENSE.txt $(WINDIR)/LICENSE.txt
	cp README.md   $(WINDIR)/README.txt
	zip -FSrT "gemforce-win-$(VERSION).zip" "$(WINDIR)"

windows-tables: $(TABLES_DIR)/table_* | $(TABLES_DIR) $(WINDIR)
	@cp -vr $(TABLES_DIR)/ $(WINDIR)/

$(WINDIR):
	mkdir -p $(WINDIR)

.PHONY: windows windows-tables

# Move
move: | $(BINDIR)
	@for exe in $(DEV_ALL); do \
		[ -e "$${exe}" ] && mv -v $${exe} "$(BINDIR)/"; \
	done; \
	: # noop so make doesn't choke

move-white: | $(BINDIR)
	@if [ -d "white" ]; then \
		find "white/" -type f -executable -exec mv -v {} "$(BINDIR)/white/" \; ; \
	fi

$(BINDIR):
	mkdir $(BINDIR)

.PHONY: move move-white

# Include deps folder
-include $(wildcard $(DEPS_DIR)/*.d)

# Compilation targets
$(DEV_ALL): %: %.cpp | $(DEPS_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MF $(DEPS_DIR)/$(<F).d -MT $@ $< $(LIBFLAGS) -o $@

$(DEPS_DIR):
	mkdir -p $(DEPS_DIR)

# Tables
tables: $(TABLES_DIR)/table_* | $(TABLES_DIR) $(BINDIR)
	@cp -vr $(TABLES_DIR)/ $(BINDIR)/
$(TABLES_DIR):
	@echo \"$(TABLES_DIR)\" directory not found. Aborting...
	@false

.PHONY: tables

# Clean
clean:
	@rm -vf $(DEV_ALL)
	@rm $(DEPS_DIR)/*

clean-white:
	@if [ -d "white" ]; then \
		find "white/" -type f -executable -delete; \
	fi

clean-windows:
	@rm -vrf $(WINDIR)

clean-all: clean clean-white clean-windows

.PHONY: clean clean-white clean-windows clean-all
