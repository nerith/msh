CC=gcc
OPTIONS= -Wall -std=gnu11
SRC_FILES=src/*.c
OUTPUT=shell
INCLUDE_DIR=.

all:
	$(CC) $(OPTIONS) $(SRC_FILES) -o $(OUTPUT) -I $(INCLUDE_DIR)
