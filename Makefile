all: server
clean:
	rm server
server:
	gcc -pthread server.c html.c pgLib.c -o server
