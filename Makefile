.POSIX:

SHELL := /bin/zsh
UNAME := $(shell uname -s)
ARCH  := $(shell uname -m)

# Installation prefix (only used for non-bundle builds)
PREFIX := /usr/local

# Detect Homebrew prefix (Apple Silicon vs Intel)
BREW_PREFIX := $(shell brew --prefix 2>/dev/null || echo "/usr/local")

# Build type: debug, debugoptimized, release, minsize
BUILDTYPE ?= debugoptimized

# Build directory
BUILD_DIR ?= build

# Bundle build directory (for .app packaging)
BUNDLE_DIR ?= build_static

# Default meson options for macOS
# - default_library=static: statically link dependencies where possible
# - local_boost=true: use bundled boost subproject (avoids brew boost issues)
# - build_osx_bundle=true: configure paths as Aegisub.app/Contents/...
# - --force-fallback-for=ffms2: build ffms2 from subproject (more reliable)
MACOS_ARGS ?= -Ddefault_library=static -Dlocal_boost=true -Dbuild_osx_bundle=true --force-fallback-for=ffms2

# For non-bundle development builds (faster, no .app packaging)
DEV_ARGS ?= -Ddefault_library=static -Dlocal_boost=true

# Number of parallel jobs
JOBS ?= $(shell sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Python interpreter
PYTHON ?= python3

###############################################################################
# Prerequisite Detection & Environment
###############################################################################

# Ensure pkg-config can find Homebrew-installed ICU
export PKG_CONFIG_PATH := $(BREW_PREFIX)/opt/icu4c/lib/pkgconfig:$(PKG_CONFIG_PATH)
export LDFLAGS := -L$(BREW_PREFIX)/opt/icu4c/lib
export CPPFLAGS := -I$(BREW_PREFIX)/opt/icu4c/include

.PHONY: help
help:
	@echo 'Aegisub Build System (macOS)'
	@echo ''
	@echo 'Quick Start:'
	@echo '  make deps          Install all Homebrew dependencies'
	@echo '  make setup         Configure the build'
	@echo '  make build         Build the binary'
	@echo '  make               Same as make build (default target)'
	@echo '  make test          Run tests'
	@echo ''
	@echo 'Bundle/DMG (distribution):'
	@echo '  make bundle-setup  Configure for .app bundle'
	@echo '  make bundle         Build and create Aegisub.app'
	@echo '  make dmg            Create Aegisub-<version>.dmg from the app bundle'
	@echo '  make dist           Full clean build + bundle + dmg (one-step release)'
	@echo ''
	@echo 'Development:'
	@echo '  make dev-setup     Configure for development (no bundle paths)'
	@echo '  make dev            Build without .app packaging (faster)'
	@echo '  make fmt            Run clang-format if available'
	@echo ''
	@echo 'Utility:'
	@echo '  make check-env     Print detected environment variables'
	@echo '  make clean         Remove only build directory'
	@echo '  make distclean     Remove build directories and meson info'
	@echo '  make re            Shortcut: distclean + setup + build'
	@echo '  make uninstall     Remove installed files (non-bundle only)'

###############################################################################
# Default target
###############################################################################

.PHONY: default
default: build

###############################################################################
# Dependencies (Homebrew)
###############################################################################

BREW_DEPS = cmake ninja pkg-config libass boost zlib ffms2 fftw hunspell uchardet
BREW_OPT_DEPS = luajit gettext

.PHONY: deps
deps:
	@echo "==> Installing required Homebrew packages..."
	brew install $(BREW_DEPS) $(BREW_OPT_DEPS)
	@echo "==> Verifying ICU is linked..."
	@if [ ! -d "$(BREW_PREFIX)/opt/icu4c/lib/pkgconfig" ]; then \
		echo "WARNING: ICU pkg-config not found. Try: brew link icu4c --force"; \
	fi
	@echo "==> Checking Python/meson..."
	which $(PYTHON) || $(PYTHON) --version || (echo "Install Python 3"; exit 1)
	$(PYTHON) -m pip install meson 2>/dev/null || true
	@echo "==> Done. Run 'make setup' to configure."

.PHONY: deps-check
deps-check:
	@echo "==> Checking dependencies..."
	@for pkg in $(BREW_DEPS); do \
		if brew list $$pkg &>/dev/null; then \
			echo "  [OK] $$pkg"; \
		else \
			echo "  [MISSING] $$pkg"; \
		fi; \
	done

###############################################################################
# Environment Verification
###############################################################################

.PHONY: check-env
check-env:
	@echo "System:    $(UNAME) / $(ARCH)"
	@echo "Homebrew:  $(BREW_PREFIX)"
	@echo "Python:    $(shell which $(PYTHON) 2>/dev/null || echo 'not found')"
	@echo "Meson:     $(shell meson --version 2>/dev/null || echo 'not found')"
	@echo "Ninja:     $(shell ninja --version 2>/dev/null || echo 'not found')"
	@echo "CMake:     $(shell cmake --version 2>/dev/null | head -1 || echo 'not found')"
	@echo "Jobs:      $(JOBS)"
	@echo "PKG_CONFIG_PATH: $(PKG_CONFIG_PATH)"
	@echo "LDFLAGS:   $(LDFLAGS)"
	@echo "CPPFLAGS:  $(CPPFLAGS)"

###############################################################################
# Development Build (no bundle, faster iteration)
###############################################################################

.PHONY: dev-setup
dev-setup: check-env
	@echo "==> Configuring development build..."
	meson setup $(BUILD_DIR) \
		--buildtype=$(BUILDTYPE) \
		$(DEV_ARGS)

.PHONY: dev
dev: dev-setup
	@echo "==> Building..."
	meson compile -C $(BUILD_DIR) -j$(JOBS)

.PHONY: dev-rebuild
dev-rebuild:
	@echo "==> Rebuilding..."
	meson compile -C $(BUILD_DIR) -j$(JOBS)

###############################################################################
# Standard Build (no bundle)
###############################################################################

.PHONY: setup
setup: check-env
	@echo "==> Configuring build..."
	meson setup $(BUILD_DIR) \
		--buildtype=$(BUILDTYPE) \
		$(DEV_ARGS)

.PHONY: build
build:
	@echo "==> Building..."
	meson compile -C $(BUILD_DIR) -j$(JOBS)

.PHONY: build-verbose
build-verbose:
	meson compile -C $(BUILD_DIR) -j$(JOBS) -v

###############################################################################
# Bundle (.app) Build
###############################################################################

.PHONY: bundle-setup
bundle-setup: check-env
	@echo "==> Configuring bundle build..."
	meson setup $(BUNDLE_DIR) \
		--buildtype=$(BUILDTYPE) \
		$(MACOS_ARGS)

.PHONY: bundle-build
bundle-build:
	@echo "==> Building bundle binary..."
	meson compile -C $(BUNDLE_DIR) -j$(JOBS)

.PHONY: bundle
bundle: bundle-setup bundle-build
	@echo "==> Creating Aegisub.app..."
	meson compile osx-bundle -C $(BUNDLE_DIR)

.PHONY: osx-bundle
osx-bundle: bundle

.PHONY: bundle-quick
bundle-quick:
	@echo "==> Building and creating bundle (no reconfigure)..."
	meson compile -C $(BUNDLE_DIR) -j$(JOBS)
	meson compile osx-bundle -C $(BUNDLE_DIR)

###############################################################################
# DMG Creation
###############################################################################

.PHONY: dmg
dmg:
	@echo "==> Creating DMG..."
	@if [ ! -d "$(BUNDLE_DIR)/Aegisub.app" ]; then \
		echo "ERROR: Aegisub.app not found in $(BUNDLE_DIR). Run 'make bundle' first."; \
		exit 1; \
	fi
	meson compile osx-build-dmg -C $(BUNDLE_DIR)

.PHONY: osx-build-dmg
osx-build-dmg: dmg

###############################################################################
# Full Release Build
###############################################################################

.PHONY: dist
dist: distclean bundle-setup
	@echo "==> Building and packaging..."
	meson compile -C $(BUNDLE_DIR) -j$(JOBS)
	meson compile osx-bundle -C $(BUNDLE_DIR)
	@echo "==> Creating DMG (with retries for hdiutil)..."
	@max_tries=10; i=0; \
	until meson compile osx-build-dmg -C $(BUNDLE_DIR); do \
		i=$$((i+1)); \
		if [ $$i -eq $$max_tries ]; then \
			echo "Error: osx-build-dmg failed after $$max_tries tries."; \
			exit 1; \
		fi; \
		echo "Retrying osx-build-dmg (attempt $$((i+1))/$${max_tries})..."; \
	done
	@echo "==> Release package: $(BUNDLE_DIR)/Aegisub-*.dmg"

###############################################################################
# Tests
###############################################################################

.PHONY: test
test:
	meson test -C $(BUILD_DIR) --verbose "gtest main"

.PHONY: test-bundle
test-bundle:
	meson test -C $(BUNDLE_DIR) --verbose "gtest main"

.PHONY: test-all
test-all:
	meson test -C $(BUILD_DIR) --verbose

###############################################################################
# Reconfigure / Rebuild
###############################################################################

.PHONY: reconfigure
reconfigure:
	meson setup --reconfigure $(BUILD_DIR) --buildtype=$(BUILDTYPE) $(DEV_ARGS)

.PHONY: reconfigure-bundle
reconfigure-bundle:
	meson setup --reconfigure $(BUNDLE_DIR) --buildtype=$(BUILDTYPE) $(MACOS_ARGS)

.PHONY: resolve
resolve:
	meson subprojects download && meson subprojects packagefiles --apply

###############################################################################
# Clean
###############################################################################

.PHONY: clean
clean:
	@echo "==> Cleaning..."
	rm -rf $(BUILD_DIR) $(BUNDLE_DIR) git_version.h

.PHONY: distclean
distclean:
	@echo "==> Full clean..."
	rm -rf $(BUILD_DIR) $(BUNDLE_DIR) git_version.h meson-info meson-logs

.PHONY: clean-bundle
clean-bundle:
	rm -rf $(BUNDLE_DIR)

.PHONY: clean-dmg
clean-dmg:
	rm -f $(BUNDLE_DIR)/Aegisub-*.dmg $(BUNDLE_DIR)/*_rw.dmg $(BUNDLE_DIR)/temp_dmg

###############################################################################
# Source Tarball
###############################################################################

.PHONY: tarball
tarball:
	@echo "==> Generating source tarball..."
	meson dist --include-subprojects -C $(BUILD_DIR)

###############################################################################
# Install (non-bundle only)
###############################################################################

.PHONY: install
install:
	@echo "==> Installing to $(PREFIX)..."
	meson install -C $(BUILD_DIR) --skip-subprojects luajit

.PHONY: uninstall
uninstall:
	@echo "==> Uninstalling..."
	@if [ -f "$(BUILD_DIR)/meson-info/install-log.txt" ]; then \
		cat $(BUILD_DIR)/meson-info/install-log.txt | while read f; do \
			rm -f "$$f" 2>/dev/null || true; \
		done; \
	fi

###############################################################################
# Code Quality
###############################################################################

.PHONY: lint
lint:
	@echo "==> Running clang-tidy (if available)..."
	@if command -v clang-tidy &>/dev/null; then \
		meson compile -C $(BUILD_DIR) -j$(JOBS) clang-tidy || true; \
	fi

.PHONY: fmt
fmt:
	@echo "==> Running clang-format..."
	@if command -v clang-format &>/dev/null; then \
		find libaegisub src tests -name '*.cpp' -o -name '*.h' -o -name '*.mm' | \
		xargs clang-format -i -style=file 2>/dev/null || true; \
	fi

.PHONY: check-format
check-format:
	@if command -v clang-format &>/dev/null; then \
		find libaegisub src tests -name '*.cpp' -o -name '*.h' -o -name '*.mm' | \
		xargs clang-format --dry-run --Werror -style=file 2>/dev/null || true; \
	fi

###############################################################################
# Convenience Aliases
###############################################################################

.PHONY: re
re: distclean setup build

.PHONY: rebundle
rebundle: distclean bundle-setup bundle-build bundle

.PHONY: redmg
redmg: distclean bundle-setup bundle-build bundle dmg