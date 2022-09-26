
FLAGS = -Wall -Werror -pedantic -g

all: client
	./client

client: client.c
	gcc $(FLAGS) $^ -o $@

clean:
	rm -rf client
        git rm client
