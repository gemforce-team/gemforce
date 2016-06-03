# Compiler
CC:=gcc
WINCC:=x86_64-w64-mingw32-$(CC)
CXX:=g++
WINCXX:=x86_64-w64-mingw32-$(CXX)

# Flags
CSTD:=-std=gnu11
CXXSTD:=-std=gnu++11
WFLAGS:=-Wall -Wextra
OFLAGS:=-O3 -flto -fwhole-program
IFLAGS:=-I "include/"
LFLAGS:=-lm
CFLAGS:=$(CSTD) $(WFLAGS) $(OFLAGS) $(IFLAGS) $(LFLAGS)
CXXFLAGS:=$(CXXSTD) $(WFLAGS) $(OFLAGS) $(IFLAGS) $(LFLAGS)

# Query
QUERY_LEECH=managem/leechquery
QUERY_MANAGEM=managem/mgquery-alone managem/mgquery-amps managem/mgquery-setup managem/mgquery-omnia
QUERY_CRIT=killgem/critquery
QUERY_KILLGEM=killgem/kgquery-alone killgem/kgquery-amps killgem/kgquery-setup killgem/kgquery-omnia
QUERY_MGNGEM=managem/mgquery-ngems
QUERY_KGNGEM=killgem/kgquery-ngems
QUERY_DIST=$(QUERY_LEECH) $(QUERY_MANAGEM) $(QUERY_KILLGEM)
QUERY_ALL=$(QUERY_DIST) $(QUERY_CRIT) $(QUERY_MGNGEM) $(QUERY_KGNGEM)

# Build
BUILD_LEECH=managem/leechbuild
BUILD_MANAGEM=managem/mgbuild-appr managem/mgbuild-exact managem/mgbuild-c6
BUILD_CRIT=killgem/critbuild
BUILD_KILLGEM=killgem/kgbuild-appr killgem/kgbuild-exact killgem/kgbuild-c6
BUILD_ALL=$(BUILD_LEECH) $(BUILD_MANAGEM) $(BUILD_CRIT) $(BUILD_KILLGEM)

# Combine
COMBINE_LEECH=managem/leechcombine
COMBINE_MANAGEM=managem/mgcombine-appr
COMBINE_CRIT=killgem/critcombine
COMBINE_KILLGEM=killgem/kgcombine-appr killgem/kgcombine-exact
COMBINE_ALL=$(COMBINE_LEECH) $(COMBINE_MANAGEM) $(COMBINE_CRIT) $(COMBINE_KILLGEM)

# Type aggregates
LEECH_ALL=$(QUERY_LEECH) $(BUILD_LEECH) $(COMBINE_LEECH)
MANAGEM_ALL=$(QUERY_MANAGEM) $(QUERY_MGNGEM) $(BUILD_MANAGEM) $(COMBINE_MANAGEM)
CRIT_ALL=$(QUERY_CRIT) $(BUILD_CRIT) $(COMBINE_CRIT)
KILLGEM_ALL=$(QUERY_KILLGEM) $(QUERY_KGNGEM) $(BUILD_KILLGEM) $(COMBINE_KILLGEM)

# Parser
PARSER=parser

# Folders
BINDIR:=bin
WINDIR:=$(BINDIR)/win
TABLES_DIR:=gem_tables
INCLUDE_DIR:=include

# Version
VERSION:=$(shell git describe --tags | rev | cut -d '-' -f2- | rev)

# General targets
dev: set-query set-build set-combine parser
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

.PHONY: set-query set-build set-combine set-leech set-managem set-crit set-killgem

# Windows
windows: CC:=$(WINCC)
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
	@find "managem/" -type f -executable -exec mv -v {} "$(BINDIR)/" \;

	@find "killgem/" -type f -executable -exec mv -v {} "$(BINDIR)/" \;

	@if [ -e $(PARSER) ]; then \
		mv -v $(PARSER) "$(BINDIR)/"; \
	fi

move-white: | $(BINDIR)
	@if [ -d "white" ]; then \
		find "white/" -type f -executable -exec mv -v {} "$(BINDIR)/white/" \; ; \
	fi

$(BINDIR):
	mkdir $(BINDIR)

.PHONY: move move-white

# Leech
managem/leechbuild: managem/leechbuild.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
managem/leechcombine: managem/leechcombine.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
managem/leechquery: managem/leechquery.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@

# Managem
managem/mgbuild-appr: managem/mgbuild-appr.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
managem/mgbuild-c6: managem/mgbuild-c6.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
managem/mgbuild-exact: managem/mgbuild-exact.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
managem/mgcombine-appr: managem/mgcombine-appr.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
managem/mgquery-alone: managem/mgquery-alone.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
managem/mgquery-amps: managem/mgquery-amps.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
managem/mgquery-ngems: managem/mgquery-ngems.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
managem/mgquery-omnia: managem/mgquery-omnia.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
managem/mgquery-setup: managem/mgquery-setup.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@

# Crit
killgem/critbuild: killgem/critbuild.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
killgem/critcombine: killgem/critcombine.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
killgem/critquery: killgem/critquery.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@

# Killgem
killgem/kgbuild-appr: killgem/kgbuild-appr.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
killgem/kgbuild-c6: killgem/kgbuild-c6.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
killgem/kgbuild-exact: killgem/kgbuild-exact.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
killgem/kgcombine-appr: killgem/kgcombine-appr.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
killgem/kgcombine-exact: killgem/kgcombine-exact.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
killgem/kgquery-alone: killgem/kgquery-alone.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
killgem/kgquery-amps: killgem/kgquery-amps.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
killgem/kgquery-ngems: killgem/kgquery-ngems.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
killgem/kgquery-omnia: killgem/kgquery-omnia.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@
killgem/kgquery-setup: killgem/kgquery-setup.c $(INCLUDE_DIR)/*.h
	$(CC) $(CFLAGS) $< -o $@

# Parser
parser: parser.cpp $(INCLUDE_DIR)/*.h
	$(CXX) $(CXXFLAGS) $< -o $@

# Tables
tables: $(TABLES_DIR)/table_* | $(TABLES_DIR) $(BINDIR)
	@cp -vr $(TABLES_DIR)/ $(BINDIR)/
$(TABLES_DIR):
	@echo \"$(TABLES_DIR)\" directory not found. Aborting...
	@false

.PHONY: tables

# Clean
clean:
	@rm -vf $(QUERY_ALL)
	@rm -vf $(BUILD_ALL)
	@rm -vf $(COMBINE_ALL)
	@rm -vf $(PARSER)

clean-white:
	@if [ -d "white" ]; then \
		find "white/" -type f -executable -delete; \
	fi

clean-windows:
	@rm -vrf $(WINDIR)

clean-all: clean clean-white clean-windows

.PHONY: clean clean-white clean-windows clean-all
