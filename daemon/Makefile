CC = gcc

TARGET = Daemon
BUILDDIR = build/

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

CFLAGS = -std=c99 -Wall -Wextra -pedantic
CFLAGS += -DLOGLEVEL_INFO
LIBS = -I./include/

all: $(BUILDDIR)/$(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@

$(BUILDDIR)/$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

$(shell mkdir -p $(BUILDDIR))

clean:
	rm -f $(TARGET) $(OBJ)
	rmdir $(BUILDDIR)
