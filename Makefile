#
# Makefile - makefile to build the base system
#

# Locations

# Make environment
CC=gcc
CFLAGS=-c -Wall -Wextra 
#CFLAGS += -D_XOPEN_SOURCE=500
CFLAGS += -g 
LIBS= -lpthread
# Suffix rules
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS)  -o $@ $<  
	
# Files
OBJECT_FILES= main
# Productions
all : TrainProcess

TrainProcess : $(OBJECT_FILES)
	$(CC)  $(OBJECT_FILES) -o $@ $(LIBS)
	

clean : 
	rm -f TrainProcess $(OBJECT_FILES)
	
