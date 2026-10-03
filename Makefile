CC ?= cc
CFLAGS ?= -O2 -g -std=c11 -Wall -Wextra -Werror -pedantic
CPPFLAGS ?= -Iinclude

BUILD := build
LIBOBJ := $(BUILD)/ad107_target.o
TEST := $(BUILD)/test_ad107_target
IDENTIFY := $(BUILD)/ad107-identify

.PHONY: all test clean

all: $(IDENTIFY)

$(BUILD):
	mkdir -p $(BUILD)

$(LIBOBJ): src/ad107_target.c include/ad107_target.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(IDENTIFY): tools/ad107-identify.c $(LIBOBJ) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(TEST): tests/test_ad107_target.c $(LIBOBJ) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

test: $(TEST)
	./$(TEST)

clean:
	rm -rf $(BUILD)
