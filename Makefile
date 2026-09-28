SRC = src/main.c src/server.c src/client.c lib/utils.c

NAME = lnetx

TARGET = dist/$(NAME)

CC = clang

CFLAGS = -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror -Wformat=2 -Wcast-qual -Wmissing-prototypes -Wmissing-declarations -Wstrict-prototypes

OBJ = $(SRC:%.c=build/%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p dist
	$(CC) $(CFLAGS) $(OBJ) -o $@

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)
	rm -rf build

fclean: clean
	rm -f $(TARGET)
	rm -rf dist

re: fclean
	$(MAKE) $(TARGET)

.PHONY: all clean fclean re