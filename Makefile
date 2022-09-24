
FLAGS = -Wall -Werror -pedantic --std=c++11 -g

all: a.out
	./a.out

a.out: main.cpp
	g++ $(FLAGS) $^  -l sqlite3 -o $@

clean:
	rm -rf a.out
