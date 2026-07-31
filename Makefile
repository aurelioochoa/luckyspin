CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -I./src
TARGET = gambling_game
SRC = gambling_game.cpp
HEADERS = src/colors.h src/unicode.h src/utils.h

# Installation directories (user-level, no sudo required)
PREFIX = $(HOME)/.local
BINDIR = $(PREFIX)/bin
COMMAND_NAME = lucky-spin

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

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

# Build and run Unicode test
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
	@echo "  make test-unicode - Build and run Unicode symbol test"

.PHONY: all clean run install uninstall help install-fonts install-fonts-arch install-fonts-debian install-fonts-fedora check-fonts test-unicode
