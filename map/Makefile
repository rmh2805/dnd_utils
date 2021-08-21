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
main: testSprite

#
#	Multiple Targets
#
all: tests

tests: testSprite

#
#	Executables
#
testSprite: testSprite.o sprite.o
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -o $@ $^ $(CLIBS)
	$(ECHO)

#
#	Object Files
#
%.o: %.c
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o $@ $^

fs_unix.o:
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o fs_unix.o ../common/fs_unix.c

#
#	Utils
# 
clean:
	-rm *.o > /dev/null

realclean: clean
	-rm testSprite

