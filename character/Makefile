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
main: charCreator

#
#	Multiple Targets
#
all: charCreator

#
#	Executables
#
charCreator: charCreator.o fs_unix.o list.o
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -o $@ $^ $(CLIBS)
	$(ECHO)


#
#	Object Files
#
%.o: %.c
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o $@ $^

fs_unix.o:
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o fs_unix.o ../common/fs_unix.c

list.o:
	$(CC) $(CFLAGS) $(CDEBUGFLAGS) -c -o list.o ../common/list.c

#
#	Utils
# 
clean:
	rm *.o > /dev/null

realclean: clean
	-rm charCreator
