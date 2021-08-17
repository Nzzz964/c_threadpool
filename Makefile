flags = -Wall -I./ -lpthread
objects = threadpool.o

edit: $(objects)
	gcc -o main $(flags) main.c $(objects)

threadpool.o: threadpool.c
	gcc -c threadpool.c $(flags)

clean:
	rm -r $(objects) main

build: clean edit
