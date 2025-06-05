CC = gcc

TARGET = Daemon
BUILDDIR = build/

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

CFLAGS = -Wall -Wextra -pedantic
CFLAGS += -DLOGLEVEL_INFO
LIBS = -I./include/ -lusb-1.0 -lm

all: $(BUILDDIR)/$(TARGET)

run: $(BUILDDIR)/$(TARGET)
	sudo ./$(BUILDDIR)/$(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@

$(BUILDDIR)/$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

$(shell mkdir -p $(BUILDDIR))

clean:
	rm -f $(BUILDDIR)/$(TARGET) $(OBJ)
	rmdir $(BUILDDIR)
