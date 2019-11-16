CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -pedantic -g

BIN=compiler
BIN_TEST=tests
RM=rm -f

# vvvv add source files here vvvv

BIN_SOURCE_FILES = main.c
TEST_SOURCE_FILES = test.c
COMMON_SOURCE_FILES = my_string.c scanner.c parser.c

# ^^^^ add source files here ^^^^

SOURCES_DIR = src
OBJECTS_DIR = obj


COMMON_SOURCES = $(addprefix $(SOURCES_DIR)/, COMMON_SOURCE_FILES )
BIN_SOURCES = $(addprefix $(SOURCES_DIR)/, BIN_SOURCE_FILES )
TEST_SOURCES = $(addprefix $(SOURCES_DIR)/, TEST_SOURCE_FILES )

COMMON_OBJECTS = $(addprefix $(OBJECTS_DIR)/, $(COMMON_SOURCE_FILES:.c=.o) )
BIN_OBJECTS = $(addprefix $(OBJECTS_DIR)/, $(BIN_SOURCE_FILES:.c=.o) )
TEST_OBJECTS = $(addprefix $(OBJECTS_DIR)/, $(TEST_SOURCE_FILES:.c=.o) )

all: $(BIN) $(BIN_TEST)

$(BIN): $(BIN_OBJECTS) $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_TEST): $(TEST_OBJECTS) $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) -o  $@ $^

$(OBJECTS_DIR)/%.o: $(SOURCES_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean test tests
test: $(BIN_TEST) # just a hack

clean:
	$(RM) $(OBJECTS_DIR)/*.o $(BIN) $(BIN_TEST)

leaks: $(BIN)
	valgrind --track-origins=yes --leak-check=full --show-reachable=yes ./$(EXECUTABLE) $(CMDLINE)