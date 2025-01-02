all: 
	gcc -Wall -g -o server server.c 
	gcc -Wall -g -o client client.c


clean:
	rm -f server client

