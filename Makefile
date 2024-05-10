TARGET := snake

BUILD_DIR := build
SRC_DIRS := src $(shell find vendor -type d -name src)

SRCS := $(shell find $(SRC_DIRS) -name '*.c')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

CC = clang

INC_DIRS := $(shell find vendor -name include)
INC_FLAGS := $(addprefix -I, $(INC_DIRS))

CFLAGS := -g -Wall -std=c23 $(INC_FLAGS)
LDFLAGS := -g -std=c23 -lglfw -lGL

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ 

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)
