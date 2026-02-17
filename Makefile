SRCS := $(wildcard srcs/*.c) \
				$(wildcard srcs/*/*.c) \
				glad/src/glad.c
OBJS := $(patsubst srcs/%.c,objs/%.o,$(SRCS))
CC   := gcc
CFLAGS := -Wall -Wextra -Werror -O2
INC = ./include
INCFLAGS := -lglfw -lGL
NAME := cycles

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $^ $(INCFLAGS) -I$(INC) -o $@

objs/%.o: srcs/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

objs:
	mkdir -p $@

clean:
	rm -rf objs/ main

re: clean all

.PHONY: all clean re
