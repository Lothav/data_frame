#------------------------------------------------------------------------------

SOURCE=main.cpp Module.h
PROGRAM=emulator

MYLIBRARIES=pthread
CC=g++

#------------------------------------------------------------------------------

all: $(PROGRAM)

$(PROGRAM): $(SOURCE)
	$(CC) $(SOURCE) -o$(PROGRAM) -l$(MYLIBRARIES) -std=c++14

clean:
	rm -f $(PROGRAM)






