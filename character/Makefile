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
charCreator: charCreator.o charData.o list.o dispBase.o stringUtils.o dispChar.o
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
	rm *.o > /dev/null

realclean: clean
	-rm charCreator
