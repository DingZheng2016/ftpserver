default:main

main:main.o server.o utils.o handler.o
	gcc main.o server.o utils.o handler.o -o server -Wall

main.o: main.c
	gcc -c main.c -o main.o -Wall

server.o:server.c server.h
	gcc -c server.c -o server.o -Wall

utils.o:utils.c utils.h
	gcc -c utils.c -o utils.o -Wall

handler.o: handler.c handler.h
	gcc -c handler.c -o handler.o -Wall

clean:
	rm -f *.o
