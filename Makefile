CC := gcc
SRCD := src
BLDD := build
INCD := include

ALL_SRCF := $(shell find $(SRCD) -type f -name *.c)
ALL_OBJF := $(patsubst $(SRCD)/%,$(BLDD)/%,$(ALL_SRCF:.c=.o))
FUNC_FILES := $(filter-out build/main.o, $(ALL_OBJF))
INC := -I $(INCD)

EXEC := filesec

CFLAGS := -Wall -Werror
DFLAGS := -g -DDEBUG
STD := -std=gnu11

LIBS := -lreadline -pthread -ljansson

CFLAGS += $(STD)

.PHONY: clean all

debug: CFLAGS += -g -DDEBUG
debug: all

all: setup $(EXEC)

setup:
	mkdir -p build

$(EXEC): $(ALL_OBJF)
	$(CC) $^ -o $@

$(BLDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	$(RM) -r $(BLDD) $(EXEC)