CXX = g++
CXXSTD = -std=c++11
WARNINGS = -Wall -Wextra
INCLUDES = -I./src
CXXFLAGS = $(CXXSTD) $(WARNINGS) -O2 $(INCLUDES)

TARGET = gambling_game
SRC = gambling_game.cpp

TEST_TARGET = tests/run_tests
TEST_SRC = tests/test_main.cpp

# Installation directories (user-level, no sudo required)
PREFIX = $(HOME)/.local
BINDIR = $(PREFIX)/bin
COMMAND_NAME = lucky-spin

all: $(TARGET)

# -MMD -MP emits a .d file listing every header the source actually includes,
# so edits to src/*.h and src/core/*.h trigger a rebuild automatically. This
# replaces the old hand-maintained HEADERS list, which had drifted out of date.
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -MMD -MP -o $(TARGET) $(SRC)

-include $(TARGET).d
-include gambling_game.d

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

# Build and run the core unit tests (text width, money, RNG, deck, save paths)
test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC)
	$(CXX) $(CXXFLAGS) -MMD -MP -o $(TEST_TARGET) $(TEST_SRC)

-include tests/run_tests.d

# Run the tests under AddressSanitizer + UndefinedBehaviorSanitizer
test-asan:
	$(CXX) $(CXXSTD) $(WARNINGS) $(INCLUDES) -O1 -g \
		-fsanitize=address,undefined -fno-omit-frame-pointer \
		-o tests/run_tests_asan $(TEST_SRC)
	./tests/run_tests_asan
	@rm -f tests/run_tests_asan

# ---------------------------------------------------------------------------
# Development builds
# ---------------------------------------------------------------------------

# Unoptimized build with debug symbols, for gdb
debug:
	$(CXX) $(CXXSTD) $(WARNINGS) $(INCLUDES) -O0 -g -DDEBUG -o $(TARGET)_debug $(SRC)
	@echo "Built $(TARGET)_debug - run with: gdb ./$(TARGET)_debug"

# Build the game with sanitizers enabled
sanitize:
	$(CXX) $(CXXSTD) $(WARNINGS) $(INCLUDES) -O1 -g \
		-fsanitize=address,undefined -fno-omit-frame-pointer \
		-o $(TARGET)_asan $(SRC)
	@echo "Built $(TARGET)_asan"

clean:
	rm -f $(TARGET) $(TARGET)_debug $(TARGET)_asan
	rm -f test_unicode
	rm -f $(TEST_TARGET) tests/run_tests_asan
	rm -f *.o *.d tests/*.d

run: $(TARGET)
	./$(TARGET)

# Unicode font dependencies for proper symbol display
# Arch Linux (pacman)
install-fonts-arch:
	@echo "Installing Unicode fonts for Arch Linux..."
	sudo pacman -S --needed --noconfirm \
		noto-fonts \
		noto-fonts-cjk \
		noto-fonts-emoji \
		noto-fonts-extra
	@echo "Fonts installed. You may need to restart your terminal."

# Debian/Ubuntu (apt)
install-fonts-debian:
	@echo "Installing Unicode fonts for Debian/Ubuntu..."
	sudo apt-get update
	sudo apt-get install -y \
		fonts-noto \
		fonts-noto-cjk \
		fonts-noto-color-emoji \
		fonts-noto-extra
	@echo "Fonts installed. You may need to restart your terminal."

# Fedora/RHEL (dnf)
install-fonts-fedora:
	@echo "Installing Unicode fonts for Fedora/RHEL..."
	sudo dnf install -y \
		google-noto-fonts-common \
		google-noto-cjk-fonts \
		google-noto-emoji-fonts \
		google-noto-sans-fonts \
		google-noto-serif-fonts
	@echo "Fonts installed. You may need to restart your terminal."

# Auto-detect distro and install fonts
install-fonts:
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
check-fonts:
	@echo "Checking for Noto fonts..."
	@if fc-list | grep -qi "noto"; then \
		echo "✓ Noto fonts found:"; \
		fc-list | grep -i noto | head -5; \
		echo "..."; \
	else \
		echo "✗ Noto fonts not found. Run 'make install-fonts' to install."; \
	fi

# Build and run the Unicode font/rendering test.
# Note: this is a VISUAL check - it cannot fail automatically. For assertions
# on display width, use `make test` instead.
test-unicode: test_unicode.cpp
	$(CXX) $(CXXFLAGS) -o test_unicode test_unicode.cpp
	./test_unicode
	rm -f test_unicode

# Install game as a command (user-level, no sudo required)
install: $(TARGET)
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
uninstall:
	@echo "Uninstalling $(COMMAND_NAME)..."
	@if [ -f $(BINDIR)/$(COMMAND_NAME) ]; then \
		rm -f $(BINDIR)/$(COMMAND_NAME); \
		echo "✓ $(COMMAND_NAME) has been uninstalled."; \
	else \
		echo "$(COMMAND_NAME) is not installed."; \
	fi

# Show help
help:
	@echo "Lucky Spin - Gambling Game"
	@echo ""
	@echo "Build targets:"
	@echo "  make              - Build the game"
	@echo "  make run          - Build and run the game"
	@echo "  make debug        - Unoptimized build with debug symbols"
	@echo "  make sanitize     - Build with ASan + UBSan"
	@echo "  make clean        - Remove build files"
	@echo ""
	@echo "Installation targets:"
	@echo "  make install      - Install game as '$(COMMAND_NAME)' command"
	@echo "  make uninstall    - Remove the installed command"
	@echo ""
	@echo "Font targets:"
	@echo "  make install-fonts      - Auto-detect distro and install Unicode fonts"
	@echo "  make install-fonts-arch - Install fonts on Arch Linux"
	@echo "  make install-fonts-debian - Install fonts on Debian/Ubuntu"
	@echo "  make install-fonts-fedora - Install fonts on Fedora/RHEL"
	@echo "  make check-fonts        - Check if Unicode fonts are installed"
	@echo ""
	@echo "Test targets:"
	@echo "  make test         - Run core unit tests (width, money, RNG, deck)"
	@echo "  make test-asan    - Run unit tests under ASan + UBSan"
	@echo "  make test-unicode - Visual Unicode symbol check"

.PHONY: all clean run install uninstall help \
	install-fonts install-fonts-arch install-fonts-debian install-fonts-fedora \
	check-fonts test-unicode test test-asan debug sanitize
