all: socket_server socket_client socket_client_read socket_client_chatroom

socket_server: socket_server.c
	gcc socket_server.c -lpthread -o socket_server
socket_client: socket_client.c
	gcc socket_client.c -o socket_client
socket_client_read: socket_client_read.c
	gcc socket_client_read.c -o socket_client_read
socket_client_chatroom: socket_client_chatroom.c
	gcc socket_client_chatroom.c -o socket_client_chatroom
clean:
	rm -rf socket_server socket_client socket_client_read socket_client_chatroom
