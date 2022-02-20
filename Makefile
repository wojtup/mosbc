# Alas, all of my editors went insane trying to highlight this file :(
CC = gcc
STRIP = strip
CFLAGS = -I include -g -Wall -Wextra -Wpedantic
# Object targets
OBJ_UTIL = obj/util/compiletarget.o obj/util/table.o obj/util/util.o \
	obj/util/disassembler.o
OBJ_FRONTEND = obj/front/parser.o obj/front/keyword_parser.o obj/front/lexer.o
OBJ_BACKEND = obj/back/codegen.o obj/back/runtime.o obj/back/keyword.o \
	obj/back/expression.o
OBJ = obj/main.o $(OBJ_BACKEND) $(OBJ_FRONTEND) $(OBJ_UTIL)

# If no target is provided, run release
all: release

# Release enables all optimizations
release: CFLAGS = -I include -O2 -Wall -Wextra -Wpedantic
release: bin/mosbc.exe
	$(info [36mCompiled $^ (Release build)[0m)
	@$(STRIP) $(^)

# Debug compile
debug: CFLAGS = -I include -g -Wall -Wextra -Wpedantic
debug: bin/mosbc.exe
	$(info [36mCompiled $(^) (Debug build)[0m)

# Main target, compile normally
bin/mosbc.exe: $(OBJ)
	$(info [32mBuilding $@[0m)
	@$(CC) $(^) -o $(@)

# Compile all object files
obj/%.o: src/%.c
	$(info [35mBuilding $@[0m)
	@$(CC) $(CFLAGS) $(^) -o $(@) -c

# Clean rule, for Windows
wincln:
	$(info [33mCleaning binaries[0m)
	@del obj\main.o
	@del obj\back\*.o
	@del obj\front\*.o
	@del obj\util\*.o
	@del bin\mosbc.exe

# Clean rule, for Linux (*nixes in general)
nixcln:
	$(info [33mCleaning binaries[0m)
	@rm $(OBJ)
	@rm bin/mosbc.exe
