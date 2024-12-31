all: 
	gcc -Wall -g -o server server_with_select.c 
	gcc -Wall -g -o client client.c


clean:
	rm -f server client

