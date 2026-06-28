CC = gcc
CFLAGS = -Wall

SERVER_SRC = src/server.c src/network.c src/auth.c src/file_db.c src/ipc.c \
             src/sync.c
CLIENT_SRC = src/client.c src/network.c

all: app server client

app:
	$(CC) $(CFLAGS) src/main.c src/auth.c src/file_db.c src/ipc.c src/sync.c \
	src/dispatcher.c src/agent.c src/customer.c -o app -lpthread

server: $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o server -lpthread

client: $(CLIENT_SRC)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o client

clean:
	rm -f app server client

run:
	./app

run-server:
	./server

run-client:
	./client