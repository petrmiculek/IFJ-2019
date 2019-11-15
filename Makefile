CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -pedantic -g

BIN=compiler
BIN_TEST=tests
RM=rm -f

# .c -> sources
# .h -> deps (dependencies)

# DEPS = my_string.h scanner.h parser.h
BIN_SOURCES = main.c
TEST_SOURCES = test.c
COMMON_SOURCES = my_string.c scanner.c parser.c

COMMON_OBJECTS = $(COMMON_SOURCES:.c=.o)
BIN_OBJ = $(BIN_SOURCES:.c=.o)
TEST_OBJ = $(TEST_SOURCES:.c=.o)

all: $(BIN)

%.o: $(COMMON_SOURCES) ($BIN_SOURCES) ($TEST_SOURCES)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN): $(BIN_OBJ) $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_TEST): $(TEST_OBJ) $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) -o  $@ $^

# $(SOURCES) $(TEST_SOURCES)
.PHONY: clean test tests
test:$(BIN_TEST) # just a hack

clean:
	$(RM) *.o $(BIN) $(BIN_TEST)

leaks: $(BIN)
	valgrind --track-origins=yes --leak-check=full --show-reachable=yes ./$(EXECUTABLE) $(CMDLINE)
