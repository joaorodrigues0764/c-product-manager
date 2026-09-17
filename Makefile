CC = gcc
CFLAGS = -O3 -std=c17 -pedantic -Wall -Wextra -Werror
CPPFLAGS = -Iinclude
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = c-billing-system
TEST_DIR = tests

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

test: $(TARGET)
	@set -e; \
	for input in $(TEST_DIR)/*.in; do \
		name=$$(basename $$input .in); \
		output=$$(mktemp); \
		trap 'rm -f "$$output"' EXIT INT TERM; \
		./$(TARGET) < "$$input" > "$$output"; \
		if diff -u "$(TEST_DIR)/$$name.out" "$$output" > /dev/null; then \
			echo "PASS $$name"; \
		else \
			echo "FAIL $$name"; \
			diff -u "$(TEST_DIR)/$$name.out" "$$output" || true; \
			exit 1; \
		fi; \
		rm -f "$$output"; \
		trap - EXIT INT TERM; \
	done

clean:
	rm -f $(OBJ) $(TARGET)
