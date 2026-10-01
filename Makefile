SRC = src/main.c src/server.c src/client.c lib/utils.c lib/blake3/blake3.c lib/blake3/blake3_portable.c lib/blake3/blake3_dispatch.c

NAME = lnetx

TARGET = dist/$(NAME)

CC = clang

CFLAGS = -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror -Wformat=2 -Wcast-qual -Wmissing-prototypes -Wmissing-declarations -Wstrict-prototypes

DEBUG_CFLAGS = $(CFLAGS) -g -O0

LIB_CFLAGS = -Wall -Wextra -Wpedantic -DBLAKE3_NO_SSE2 -DBLAKE3_NO_SSE41 -DBLAKE3_NO_AVX2 -DBLAKE3_NO_AVX512

OBJ = $(SRC:%.c=build/%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p dist
	$(CC) $(CFLAGS) $(OBJ) -o $@

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

build/lib/blake3/%.o: lib/blake3/%.c
	@mkdir -p $(@D)
	$(CC) $(LIB_CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)
	rm -rf build

fclean: clean
	rm -f $(TARGET)
	rm -rf dist

re: fclean
	$(MAKE) $(TARGET)

debug: CFLAGS := $(DEBUG_CFLAGS)

debug: re

.PHONY: all clean fclean re debug