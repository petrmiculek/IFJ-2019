CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -pedantic -g

BIN=compiler
RM=rm -f

# vvvv add source files here vvvv

BIN_SOURCE_FILES = main.c
COMMON_SOURCE_FILES = my_string.c scanner.c parser.c stack.c token_queue.c psa.c symtable.c code_gen.c exp_stack.c

# ^^^^ add source files here ^^^^

SOURCES_DIR = src
OBJECTS_DIR = obj

COMMON_SOURCES = $(addprefix $(SOURCES_DIR)/, COMMON_SOURCE_FILES )
BIN_SOURCES = $(addprefix $(SOURCES_DIR)/, BIN_SOURCE_FILES )

COMMON_OBJECTS = $(addprefix $(OBJECTS_DIR)/, $(COMMON_SOURCE_FILES:.c=.o) )
BIN_OBJECTS = $(addprefix $(OBJECTS_DIR)/, $(BIN_SOURCE_FILES:.c=.o) )

all: $(BIN)

$(OBJECTS_DIR):
	mkdir -p $@

$(BIN): $(BIN_OBJECTS) $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJECTS_DIR)/%.o: $(SOURCES_DIR)/%.c | $(OBJECTS_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	$(RM) $(OBJECTS_DIR)/*.o $(BIN)
