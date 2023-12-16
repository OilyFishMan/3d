CC      := clang
LINKER  ?= mold

TARGET  := build/$(shell basename $(shell pwd))
FILES   := $(shell find src -name "*.c")
OBJECTS := $(FILES:src/%.c=build/%.o)
DEPENDS := $(FILES:src/%.c=deps/%.d)

LIBS    := -lm -ltinfo -lncurses
WARN    := -Wall -Wextra -Werror
OPT     ?= 0
CFLAGS  := -Ilib -O$(OPT) $(WARN)
FINALCFLAGS := $(CFLAGS) $(LIBS) -fuse-ld=$(LINKER)

.PHONY: all run clean

all: $(TARGET)

build:
	@mkdir -p build

deps:
	@mkdir -p deps

run: $(TARGET)
	./$(TARGET)

$(TARGET): build deps $(OBJECTS)
	@if [[ -f $(TARGET) ]]; then rm $(TARGET); fi
	$(CC) $(FINALCFLAGS) -o $(TARGET) $(OBJECTS)

-include $(DEPENDS)

build/*.o: $(DEPS) Makefile
build/%.o: src/%.c
	@mkdir -p build/$(shell dirname $(shell realpath --relative-to="src" $<))
	@mkdir -p deps/$(shell dirname $(shell realpath --relative-to="src" $<))
	$(CC) -MMD -MP -MF $(<:src/%.c=deps/%.d) -MT $@ -c $(CFLAGS) -o $@ $<

clean: build deps
	@rm -r build
	@rm -r deps
