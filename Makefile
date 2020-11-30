my_server: my_server.o
	gcc -o my_server my_server.o
my_server.o: my_server.c
	gcc -c my_server.c

clean:
	rm -f my_server *.o *~
