all: 
	gcc -Wall -g -o server server_with_select.c 
	gcc -Wall -O0 -g -o client client.c


clean:
	rm -f server client

