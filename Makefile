CC := gcc
SRCD := src
BLDD := build
INCD := include

ALL_SRCF := $(shell find $(SRCD) -type f -name *.c)
ALL_OBJF := $(patsubst $(SRCD)/%,$(BLDD)/%,$(ALL_SRCF:.c=.o))
FUNC_FILES := $(filter-out build/main.o, $(ALL_OBJF))
INC := -I $(INCD)

EXEC := filesec

CFLAGS := -g -O2 -Wall -Werror
LDFLAGS := -lssl -lcrypto
STD := -std=gnu11

CFLAGS += $(STD)

.PHONY: clean all

all: setup $(EXEC)

setup:
	mkdir -p build

$(EXEC): $(ALL_OBJF)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BLDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	$(RM) -r $(BLDD) $(EXEC)