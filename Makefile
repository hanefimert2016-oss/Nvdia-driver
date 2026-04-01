# Makefile – NvidiaHackintoshDriver kext
#
# Requirements:
#   macOS with Xcode command-line tools  OR
#   A Linux/CI environment (for syntax checking / linting only)
#
# Targets:
#   all        – build the kext binary (macOS only)
#   lint       – syntax-check source files with clang
#   validate   – validate the Info.plist with plutil (macOS) or xmllint (Linux)
#   test       – run the unit tests in tests/
#   package    – create a distributable .zip
#   clean      – remove build artefacts

PRODUCT_NAME   := NvidiaHackintoshDriver
KEXT_DIR       := $(PRODUCT_NAME).kext
BINARY         := $(KEXT_DIR)/Contents/MacOS/$(PRODUCT_NAME)
INFO_PLIST     := $(KEXT_DIR)/Contents/Info.plist

SRC_DIR        := src
BUILD_DIR      := build

SOURCES        := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS        := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
TEST_SOURCES   := $(wildcard tests/*.c tests/*.cpp)

# ──────────────────────────────────────────────────────────────────────────────
# Detect toolchain
# ──────────────────────────────────────────────────────────────────────────────

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
  # Full macOS build
  SDK     := $(shell xcrun --show-sdk-path 2>/dev/null)
  CXX     := $(shell xcrun -f clang++ 2>/dev/null || echo clang++)
  CC      := $(shell xcrun -f clang   2>/dev/null || echo clang)
  KEXT_CFLAGS := \
    -arch x86_64 \
    -isysroot $(SDK) \
    -fno-exceptions -fno-rtti \
    -mkernel \
    -D__KERNEL__ \
    -DKERNEL \
    -DKERNEL_PRIVATE \
    -DDRIVER_PRIVATE \
    -DAPPLE \
    -DNeXT \
    -I$(SDK)/System/Library/Frameworks/Kernel.framework/Headers \
    -I$(SRC_DIR) \
    -Wall -Wextra -Wno-unused-parameter
  KEXT_LDFLAGS := \
    -arch x86_64 \
    -isysroot $(SDK) \
    -Xlinker -kext \
    -lkmodc++ \
    -lkmod \
    -lcc_kext
  PLUTIL := plutil -lint
else
  # Linux / CI: syntax-check only (no kernel headers)
  CXX     := clang++
  CC      := clang
  KEXT_CFLAGS := \
    -fsyntax-only \
    -fno-exceptions -fno-rtti \
    -I$(SRC_DIR) \
    -Wall -Wextra -Wno-unused-parameter \
    -DLINUX_SYNTAX_CHECK \
    -x c++ \
    --std=c++14
  KEXT_LDFLAGS :=
  PLUTIL := xmllint --noout
endif

.PHONY: all lint validate test package clean

# ──────────────────────────────────────────────────────────────────────────────
# Default target
# ──────────────────────────────────────────────────────────────────────────────

all: $(BINARY)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(KEXT_DIR)/Contents/MacOS:
	mkdir -p $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(KEXT_CFLAGS) -c $< -o $@

$(BINARY): $(OBJECTS) | $(KEXT_DIR)/Contents/MacOS
ifeq ($(UNAME_S),Darwin)
	$(CXX) $(KEXT_LDFLAGS) $(OBJECTS) -o $@
	@echo "✓  $(BINARY) built"
else
	@echo "ℹ  Cross-build skipped on Linux (syntax-only mode)"
	touch $@
endif

# ──────────────────────────────────────────────────────────────────────────────
# Lint – run clang syntax-check on every source file
# ──────────────────────────────────────────────────────────────────────────────

lint:
	@echo "=== Linting source files ==="
ifeq ($(UNAME_S),Darwin)
	@for f in $(SOURCES); do \
	  echo "  $$f"; \
	  $(CXX) $(KEXT_CFLAGS) -fsyntax-only $$f || exit 1; \
	done
else
	@for f in $(SOURCES); do \
	  echo "  $$f"; \
	  $(CXX) -fsyntax-only -fno-exceptions -fno-rtti \
	    -DLINUX_SYNTAX_CHECK -x c++ --std=c++14 \
	    -I$(SRC_DIR) -Istubs \
	    -Wall -Wextra -Wno-unused-parameter \
	    -Wno-unknown-pragmas \
	    $$f || exit 1; \
	done
endif
	@echo "✓  All files passed syntax check"

# ──────────────────────────────────────────────────────────────────────────────
# Validate Info.plist
# ──────────────────────────────────────────────────────────────────────────────

validate:
	@echo "=== Validating Info.plist ==="
	$(PLUTIL) $(INFO_PLIST)
	@echo "✓  Info.plist is valid"

# ──────────────────────────────────────────────────────────────────────────────
# Tests
# ──────────────────────────────────────────────────────────────────────────────

TEST_BIN := $(BUILD_DIR)/run_tests

test: $(TEST_BIN)
	@echo "=== Running unit tests ==="
	./$(TEST_BIN)
	@echo "✓  All tests passed"

$(TEST_BIN): $(TEST_SOURCES) | $(BUILD_DIR)
	$(CC) -std=c11 -Wall -Wextra -Wno-unused-parameter \
	  -I$(SRC_DIR) \
	  $(TEST_SOURCES) -o $@

# ──────────────────────────────────────────────────────────────────────────────
# Package
# ──────────────────────────────────────────────────────────────────────────────

package: all
	@echo "=== Packaging ==="
	zip -r $(PRODUCT_NAME)-$(shell date +%Y%m%d).zip \
	  $(KEXT_DIR) README.md LICENSE
	@echo "✓  Package created"

# ──────────────────────────────────────────────────────────────────────────────
# Clean
# ──────────────────────────────────────────────────────────────────────────────

clean:
	rm -rf $(BUILD_DIR) $(KEXT_DIR)/Contents/MacOS/$(PRODUCT_NAME)
	@echo "✓  Clean"
