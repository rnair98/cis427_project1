
FLAGS = -Wall -Werror -pedantic -g

all: client server

client: client.c
	gcc $(FLAGS) $^ -o $@

server: server.c
	gcc $(FLAGS) $^ -o $@

clean:
	rm -rf client; rm -rf server
    git rm client; git rm server
