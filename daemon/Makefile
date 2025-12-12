CC = gcc

TARGET = Daemon
BUILDDIR = build

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=$(BUILDDIR)/%.o)

CFLAGS = -Wall -Wextra -pedantic
CFLAGS += -DLOGLEVEL_INFO -D_REENTRANT
CFLAGS_STRICT = -Wshadow          \
  -Wincompatible-pointer-types    \
  -Wcast-align=strict             \
  -Wstrict-prototypes             \
  -Wmissing-prototypes            \
  -Wmissing-declarations          \
  -Wformat=2                      \
  -Wimplicit-fallthrough          \
  -Wlogical-op                    \
  -Wredundant-decls               \
  -Wuninitialized                 \
  -Wmaybe-uninitialized           \
  -Wunused-parameter              \
  -Wunused-but-set-variable       \
  -Wduplicated-cond               \
  -Wduplicated-branches           \
  -Wnull-dereference              \
  -Wvla                           \
  -Walloca                        \
  -fno-common                     \
  -fstack-protector-strong
LIBS = -I./include/ -I.. -lusb-1.0 -lm -lpulse -pthread

all: $(BUILDDIR)/$(TARGET)

run: $(BUILDDIR)/$(TARGET)
	./$(BUILDDIR)/$(TARGET)

$(BUILDDIR)/stb.o: src/stb.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@

$(BUILDDIR)/%.o: src/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(CFLAGS_STRICT) $(LIBS) -c $< -o $@

$(BUILDDIR)/$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(CFLAGS_STRICT) $^ $(LIBS) -o $@

$(shell mkdir -p $(BUILDDIR))

clean:
	rm -f $(BUILDDIR)/$(TARGET) $(OBJ)
	rmdir $(BUILDDIR)
