CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -I../includes -I.
LDFLAGS = -L../lib -lhttpc -lm

TEST_SRC = test_httpc.c
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_EXEC = test_httpc

all: $(TEST_EXEC)

$(TEST_EXEC): $(TEST_OBJ) ../lib/libhttpc.so
	$(CC) $(CFLAGS) -o $@ $< -L../lib -lhttpc -lm -Wl,-rpath,$(shell cd .. && pwd)/lib

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_EXEC)
	./$(TEST_EXEC)

test-valgrind: $(TEST_EXEC)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TEST_EXEC)

test-debug: $(TEST_EXEC)
	gdb ./$(TEST_EXEC)

clean:
	rm -f $(TEST_OBJ) $(TEST_EXEC)

.PHONY: all test clean test-valgrind test-debug