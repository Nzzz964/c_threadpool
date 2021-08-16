flags = -Wall -lpthread
objects = main.o threadpool.o

edit: $(objects)
	gcc -o main $(flags) $(objects)

main.o: main.c
	gcc -c main.c $(flags)

threadpool.o: threadpool.c
	gcc -c threadpool.c $(flags)

clean:
	rm -r $(objects) main

build: clean edit