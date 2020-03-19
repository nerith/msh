TARGET = msh

SRCS = src/*.c
INCLUDE = include
LIBS = -leditline

CFLAGS = -I $(INCLUDE)
CC = gcc

override CC += $(CFLAGS)

all: build

build:
	$(CC) $(LIBS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
