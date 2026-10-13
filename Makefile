PROJECT       := luckyspin
TAGLINE       := a gambling game in C++, with an embedded web GUI
HELP_VARS      = PREFIX=$(PREFIX)  COMMAND_NAME=$(COMMAND_NAME)
HELP_EXAMPLE   = make build && make run

# ─── Preamble ────────────────────────────────────────────────────────────────
# -e stops a recipe at the first failing command; -o pipefail stops a gate that
# pipes through a filter from reporting the filter's exit code instead of the
# command's — without it, a suite that fails into `tail` "passes".
SHELL         := /usr/bin/env bash
.SHELLFLAGS   := -eo pipefail -c
.DEFAULT_GOAL := help
MAKEFLAGS     += --no-print-directory

# ─── Colour ──────────────────────────────────────────────────────────────────
# MAKE_TERMOUT (GNU Make >= 4.1) is set only when stdout is a terminal, and is
# the only reliable test available here: `test -t 1` inside $(shell ...) always
# reports false, because make captures that command's stdout through a pipe.
# So `make help | less` and CI logs stay clean. NO_COLOR disables, FORCE_COLOR
# overrides both.
COLOR ?= $(if $(MAKE_TERMOUT),1,0)
ifdef NO_COLOR
  COLOR := 0
endif
ifdef FORCE_COLOR
  COLOR := 1
endif
ifeq ($(COLOR),1)
  # Real ESC bytes, so a plain `echo` renders them without needing -e.
  C_HEAD := $(shell printf '\033[1m')
  C_CMD  := $(shell printf '\033[36m')
  C_OK   := $(shell printf '\033[32m')
  C_WARN := $(shell printf '\033[33m')
  C_ERR  := $(shell printf '\033[31m')
  C_DIM  := $(shell printf '\033[2m')
  C_OFF  := $(shell printf '\033[0m')
endif

# Explains why a core verb does not apply here, then fails.
NA = @printf '  $(C_ERR)make $@$(C_OFF) does not apply to $(PROJECT).\n  $(C_DIM)%s$(C_OFF)\n\n' $(1) >&2; exit 2

# Width of the target-name column in help; widen it where names are long.
HELP_PAD ?= 18

# PROJECT, TAGLINE, HELP_VARS and HELP_EXAMPLE are interpolated into a
# single-quoted shell string below, so none of them may contain an apostrophe.
##@ General
.PHONY: help
help: ## List the available targets
	@printf '\n  $(C_HEAD)$(PROJECT)$(C_OFF) — $(TAGLINE)\n'
	@printf '  $(C_DIM)usage: make <target>$(C_OFF)\n'
	@awk 'BEGIN { FS = ":.*?## " } \
	  /^##@ / { printf "\n  $(C_HEAD)%s$(C_OFF)\n", substr($$0, 5); next } \
	  /^[a-zA-Z0-9_.-]+:.*?## / { printf "    $(C_CMD)%-$(HELP_PAD)s$(C_OFF) %s\n", $$1, $$2 }' \
	  $(MAKEFILE_LIST)
	@printf '\n  $(C_DIM)Variables:$(C_OFF) $(HELP_VARS)\n'
	@printf '  $(C_DIM)Example:$(C_OFF)   $(HELP_EXAMPLE)\n\n'

# ─── End of the shared block ─────────────────────────────────────────────────

# Build the shared C++ engine and embed the offline frontend and artwork.

CXX = g++
CXXSTD = -std=c++11
WARNINGS = -Wall -Wextra
INCLUDES = -I./src
CXXFLAGS = $(CXXSTD) $(WARNINGS) -O2 $(INCLUDES)

TARGET = gambling_game
SRC = src/game_init.cpp src/game_economy.cpp src/game_actions.cpp \
      src/game_display.cpp \
      src/game_games.cpp src/game_shops.cpp src/game_save.cpp \
      src/game_loop.cpp \
      src/web/server.cpp src/web/api.cpp src/main.cpp
OBJ = $(SRC:.cpp=.o)

# The frontend is embedded into the binary so it stays a single file.
WEB_INDEX = web/index.html
ASSET_HEADER = src/web_assets.h
ART_HEADER = src/art_assets.h

TEST_TARGET = tests/run_tests
TEST_SRC = tests/test_main.cpp

# The test binary links every game object except main() so the headless
# action layer (game_actions.cpp) can be exercised end-to-end.
TEST_GAME_OBJ = $(filter-out src/main.o,$(OBJ))

# Installation directories (user-level, no sudo required)
PREFIX = $(HOME)/.local
BINDIR = $(PREFIX)/bin
COMMAND_NAME = lucky-spin

# Each .cpp compiles to a .o; -MMD -MP emits a .d file listing every header
# the source actually includes, so edits to src/*.h and src/core/*.h trigger
# a rebuild automatically. Unchanged objects are not recompiled.
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

# api.cpp embeds the frontend; regenerate it whenever index.html changes.
$(ASSET_HEADER): $(WEB_INDEX)
	@echo '// GENERATED FILE - rebuilt from web/index.html by make. Do not edit.' > $@
	@echo '#pragma once' >> $@
	@echo 'namespace assets {' >> $@
	@printf 'const char* WEB_INDEX_HTML = R"WEBASSET(' >> $@
	@cat $(WEB_INDEX) >> $@
	@printf ')WEBASSET";\n}' >> $@
	@echo >> $@

src/web/api.o: $(ASSET_HEADER) $(ART_HEADER)

$(ART_HEADER): web/assets/casino.png scripts/embed-art.py
	python3 scripts/embed-art.py $< $@

-include $(SRC:.cpp=.d)

##@ Development
.PHONY: build
build: $(TARGET) ## Compile the game

.PHONY: dev
dev: debug ## Unoptimised build with debug symbols, for iterating

##@ Gates
.PHONY: check
check: test test-tui check-fonts ## Engine, terminal, and font checks

.PHONY: verify
verify: check build ## Full gate: checks and an optimised build

.PHONY: fmt
fmt: ## Not applicable here
	$(call NA,'No formatter is configured. Add clang-format if you want one.')

##@ Setup
.PHONY: setup
setup: install-fonts ## First run: install the fonts the GUI needs

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

# Build and run the core unit tests (text width, money, RNG, deck, save paths)
test: $(TEST_TARGET) ## Build and run the unit test suite
	@./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC) $(TEST_GAME_OBJ)
	$(CXX) $(CXXFLAGS) -MMD -MP -o $(TEST_TARGET) $(TEST_SRC) $(TEST_GAME_OBJ)

-include tests/run_tests.d

# Run the tests under AddressSanitizer + UndefinedBehaviorSanitizer.
# Game sources are recompiled here so the whole program is instrumented.
test-asan: $(ASSET_HEADER) $(ART_HEADER) ## Run the tests under AddressSanitizer
	$(CXX) $(CXXSTD) $(WARNINGS) $(INCLUDES) -O1 -g \
		-fsanitize=address,undefined -fno-omit-frame-pointer \
		-o tests/run_tests_asan $(TEST_SRC) $(TEST_GAME_OBJ:.o=.cpp)
	./tests/run_tests_asan
	@rm -f tests/run_tests_asan

# Optional interaction checks; browser tests require Playwright.
.PHONY: test-tui test-browser
test-tui: $(TARGET) ## Verify terminal layouts and navigation
	python3 tests/test-tui.py

test-browser: $(TARGET) ## Verify the live GUI with Playwright
	node tests/browser.mjs

# ---------------------------------------------------------------------------
# Development builds
# ---------------------------------------------------------------------------

# Unoptimized build with debug symbols, for gdb
debug: $(ASSET_HEADER) $(ART_HEADER) ## Unoptimised build with debug symbols
	$(CXX) $(CXXSTD) $(WARNINGS) $(INCLUDES) -O0 -g -DDEBUG -o $(TARGET)_debug $(SRC)
	@echo "Built $(TARGET)_debug - run with: gdb ./$(TARGET)_debug"

# Build the game with sanitizers enabled
sanitize: $(ASSET_HEADER) $(ART_HEADER) ## Build with ASan and UBSan
	$(CXX) $(CXXSTD) $(WARNINGS) $(INCLUDES) -O1 -g \
		-fsanitize=address,undefined -fno-omit-frame-pointer \
		-o $(TARGET)_asan $(SRC)
	@echo "Built $(TARGET)_asan"

##@ Cleaning
clean: ## Remove build files
	rm -f $(TARGET) $(TARGET)_debug $(TARGET)_asan
	rm -f test_unicode
	rm -f $(TEST_TARGET) tests/run_tests_asan
	rm -f src/*.o src/*.d *.o *.d tests/*.d src/web/*.o src/web/*.d
	rm -f $(ASSET_HEADER) $(ART_HEADER)

##@ Running
run: $(TARGET) ## Build and run the game
	./$(TARGET)

# Web GUI on the default port (http://127.0.0.1:8787/)
gui: $(TARGET) ## Build and serve the web GUI (http://127.0.0.1:8787/)
	./$(TARGET) --gui

##@ Fonts
# Unicode font dependencies for proper symbol display
# Arch Linux (pacman)
install-fonts-arch: ## Install the GUI fonts on Arch
	@echo "Installing Unicode fonts for Arch Linux..."
	sudo pacman -S --needed --noconfirm \
		noto-fonts \
		noto-fonts-cjk \
		noto-fonts-emoji \
		noto-fonts-extra
	@echo "Fonts installed. You may need to restart your terminal."

# Debian/Ubuntu (apt)
install-fonts-debian: ## Install the GUI fonts on Debian
	@echo "Installing Unicode fonts for Debian/Ubuntu..."
	sudo apt-get update
	sudo apt-get install -y \
		fonts-noto \
		fonts-noto-cjk \
		fonts-noto-color-emoji \
		fonts-noto-extra
	@echo "Fonts installed. You may need to restart your terminal."

# Fedora/RHEL (dnf)
install-fonts-fedora: ## Install the GUI fonts on Fedora
	@echo "Installing Unicode fonts for Fedora/RHEL..."
	sudo dnf install -y \
		google-noto-fonts-common \
		google-noto-cjk-fonts \
		google-noto-emoji-fonts \
		google-noto-sans-fonts \
		google-noto-serif-fonts
	@echo "Fonts installed. You may need to restart your terminal."

# Auto-detect distro and install fonts
install-fonts: ## Install the GUI fonts for your distribution
	@if command -v pacman >/dev/null 2>&1; then \
		$(MAKE) install-fonts-arch; \
	elif command -v apt-get >/dev/null 2>&1; then \
		$(MAKE) install-fonts-debian; \
	elif command -v dnf >/dev/null 2>&1; then \
		$(MAKE) install-fonts-fedora; \
	else \
		echo "Unsupported package manager. Please install Noto fonts manually:"; \
		echo "  - noto-fonts"; \
		echo "  - noto-fonts-cjk"; \
		echo "  - noto-fonts-emoji"; \
		exit 1; \
	fi

# Check if Unicode fonts are installed
check-fonts: ## Check that the GUI fonts are present
	@echo "Checking for Noto fonts..."
	@if fc-list | grep -qi "noto"; then \
		echo "✓ Noto fonts found:"; \
		fc-list | awk 'tolower($$0) ~ /noto/ { if (n++ < 5) print }'; \
		echo "..."; \
	else \
		echo "✗ Noto fonts not found. Run 'make install-fonts' to install."; \
	fi

# Build and run the Unicode font/rendering test.
# Note: this is a VISUAL check - it cannot fail automatically. For assertions
# on display width, use `make test` instead.
test-unicode: test_unicode.cpp ## Build and run the Unicode rendering check
	$(CXX) $(CXXFLAGS) -o test_unicode test_unicode.cpp
	./test_unicode
	rm -f test_unicode

##@ Installation
# Install game as a command (user-level, no sudo required)
install: $(TARGET) ## Install the game as the lucky-spin command
	@echo "Installing $(COMMAND_NAME) to $(BINDIR)..."
	@mkdir -p $(BINDIR)
	@cp $(TARGET) $(BINDIR)/$(COMMAND_NAME)
	@chmod +x $(BINDIR)/$(COMMAND_NAME)
	@echo ""
	@echo "✓ Installed successfully!"
	@echo "  Command: $(COMMAND_NAME)"
	@echo "  Location: $(BINDIR)/$(COMMAND_NAME)"
	@echo ""
	@if echo "$$PATH" | grep -q "$(BINDIR)"; then \
		echo "You can now run the game by typing: $(COMMAND_NAME)"; \
	else \
		echo "NOTE: $(BINDIR) is not in your PATH."; \
		echo "Add this line to your ~/.bashrc or ~/.zshrc:"; \
		echo "  export PATH=\"\$$HOME/.local/bin:\$$PATH\""; \
		echo ""; \
		echo "Then run: source ~/.bashrc  (or restart your terminal)"; \
	fi

# Uninstall game command
uninstall: ## Remove the installed command
	@echo "Uninstalling $(COMMAND_NAME)..."
	@if [ -f $(BINDIR)/$(COMMAND_NAME) ]; then \
		rm -f $(BINDIR)/$(COMMAND_NAME); \
		echo "✓ $(COMMAND_NAME) has been uninstalled."; \
	else \
		echo "$(COMMAND_NAME) is not installed."; \
	fi

.PHONY: distclean
distclean: clean ## Same as clean: nothing is installed outside $(PREFIX)

# ─── Compatibility aliases ───────────────────────────────────────────────────
# `all` was the default goal and compiled; it still compiles when asked by name.
.PHONY: all
all: build

.PHONY: run gui install uninstall debug sanitize test test-asan check-fonts \
        install-fonts install-fonts-arch install-fonts-debian install-fonts-fedora
