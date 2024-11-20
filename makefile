all: 
	gcc -Wall -o server server.c 
	gcc -Wall -o client client.c


clean:
	rm -f server client

