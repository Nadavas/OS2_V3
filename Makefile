CC = g++
CFLAGS = -std=c++11 -Wall -Werror -pedantic-errors -DNDEBUG -pthread -g

main: main.o bank.o
	$(CC) $(CFLAGS) main.o bank.o -o main

main.o: main.cpp bank.h
	$(CC) $(CFLAGS) -c main.cpp

bank.o: bank.cpp bank.h
	$(CC) $(CFLAGS) -c bank.cpp

clean:
	rm -f main *.o
