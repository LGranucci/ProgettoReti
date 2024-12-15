all: 
	gcc -Wall -o server server_with_select.c 
	gcc -Wall -o client client.c


clean:
	rm -f server client

