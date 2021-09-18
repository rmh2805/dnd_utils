#
#	Commands
#

# Compilers
CC=gcc

# Misc
ECHO=@echo

#
#	Misc Variables
#

# C Flags
CLIBS=-lm -lncurses
CFLAGS=-Wall -std=c99 -Wextra -pedantic -ggdb

#
#	Main Target
#
main: mkWorldMap

#
#	Multiple Targets
#
all: mkWorldMap

#
#	Executables
#
mkWorldMap: mkWorldMap.o map.o sprite.o tile.o list.o
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -o $@ $^

#
#	Object Files
#
%.o: %.c
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o $@ $^

# Common Code
fs_unix.o: ../common/fs_unix.c
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o $@ $^

list.o: ../common/list.c
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o $@ $^

dispBase.o: ../common/dispBase.c
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o $@ $^

# Map Code
map.o: ../map/map.c
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o $@ $^

sprite.o: ../map/sprite.c
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o $@ $^

tile.o: ../map/tile.c
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o $@ $^

mapDisp.o: ../map/mapDisp.c
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o $@ $^

#
#	Utils
# 
clean:
	-rm *.o > /dev/null

realclean: clean
	-rm mkWorldMap

