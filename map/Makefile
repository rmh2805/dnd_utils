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
main: makeSprite

#
#	Multiple Targets
#
all: tests makeSprite

tests: testSprite

#
#	Executables
#
makeSprite: makeSprite.o sprite.o dispBase.o list.o
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -o $@ $^ $(CLIBS)
	$(ECHO)

testSprite: testSprite.o sprite.o
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

#
#	Utils
# 
clean:
	-rm *.o > /dev/null

realclean: clean
	-rm testSprite
	-rm makeSprite

