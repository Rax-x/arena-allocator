CC := gcc
CFLAGS := -Wall -Wextra -ggdb -std=c99 -pedantic

test_sources := $(wildcard tests/*.c)
test_executables := $(test_sources:.c=)

.PHONY: test clean

test: $(test_executables)
	@$(foreach test_executable, $?, ./$(test_executable);)

%: %.c tests/test.h
	@$(CC) $(CFLAGS) $< -o $@

clean:
	@rm -rf $(test_executables)
