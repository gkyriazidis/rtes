SHELL := /bin/bash

# ==================================================
# COMMANDS

CC = gcc -O3
RM = rm -f


# ==================================================
# LISTS

SRCS = $(wildcard *.c)
PROGS = $(patsubst %.c,%,$(SRCS))
all: $(PROGS)

# ==================================================
# FUNCTIONS

# compile
%: %.c
	$(CC) -o $@ $<
	@echo "Compiled programs"

# purge
purge:
	$(RM) $(PROGS)
	@echo "Purged executables"
