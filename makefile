all: socket_server socket_client

socket_server: socket_server.c
	gcc socket_server.c -lpthread -o socket_server
socket_client: socket_client.c
	gcc socket_client.c -lpthread -o socket_client
clean:
	rm -rf socket_server socket_client
