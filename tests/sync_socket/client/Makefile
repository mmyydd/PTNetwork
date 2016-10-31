


all: main


main : main.o
	gcc -o main -lpthread -luv -lptnetwork main.o

main.o : main.c
	gcc -c main.c
