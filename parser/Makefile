#Finnian Copp
#0928913
CC = gcc
LIBS = -Iinclude -LCalendarParser.h -Lfunctions.h -LLinkedListAPI.h
SRCS = src/functions.c src/CalendarParser.c src/LinkedList.c
FLAGS = -Wall -pedantic -std=c11 -g


all: parser

list: LinkedList.o
	$(CC) -shared LinkedList.o -o bin/liblist.so

LinkedList.o: src/LinkedList.c
	$(CC) src/LinkedList.c -Iinclude $(FLAGS) -c -fpic -o LinkedList.o

parser: CalendarParser.o
	$(CC) -shared CalendarParser.o LinkedList.o functions.o -o ../libcal.so

CalendarParser.o: $(SRCS)
	$(CC) $(SRCS) $(LIBS) $(FLAGS) -c -fpic #-o bin/CalendarParser.o

clean:
	rm -rf *.o *.so bin/*.o bin/*.so core

#test: src/main.c parser
	#$(CC) src/main.c bin/libcal.so $(LIBS) $(FLAGS) -o test #main.o
	#$(CC) main.o -Lbin -lcal  -o test
