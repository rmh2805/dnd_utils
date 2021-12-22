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
main: playMap

#
#	Multiple Targets
#
all: tests makeSprite makeMap randMap dispMap

tests: testSprite

#
#	Executables
#
makeMap: makeMap.o sprite.o tile.o map.o list.o dispBase.o mapDisp.o
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -o $@ $^ $(CLIBS)
	$(ECHO)

randMap: randMap.o sprite.o tile.o map.o list.o
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -o $@ $^ $(CLIBS)
	$(ECHO)

makeSprite: makeSprite.o sprite.o tile.o list.o dispBase.o mapDisp.o 
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -o $@ $^ $(CLIBS)
	$(ECHO)

dispMap: dispMap.o sprite.o tile.o map.o list.o dispBase.o mapDisp.o
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -o $@ $^ $(CLIBS)
	$(ECHO)

playMap: playMap.o sprite.o tile.o map.o list.o dispBase.o mapDisp.o
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -o $@ $^ $(CLIBS)
	$(ECHO)

testSprite: testSprite.o sprite.o list.o
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -o $@ $^ $(CLIBS)
	$(ECHO)

#
#	Object Files
#
%.o: %.c
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o $@ $^

fs_unix.o: ../common/fs_unix.c
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o $@ $^

list.o: ../common/list.c
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o $@ $^

dispBase.o: ../common/dispBase.c
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o $@ $^

#
#	Utils
# 
clean:
	-rm *.o > /dev/null

testClean: clean
	-rm testSprite

realclean: clean testClean
	-rm makeSprite
	-rm makeMap
	-rm randMap
	-rm dispMap

