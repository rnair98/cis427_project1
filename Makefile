
FLAGS = -Wall -Werror -pedantic -g

all: client server

client: client.c
	gcc $(FLAGS) $^ -o $@

server: server.c
	gcc $(FLAGS) $^ -o $@ server_functions.c -l sqlite3

clean:
	rm -rf client; rm -rf server
