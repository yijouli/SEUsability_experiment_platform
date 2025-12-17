#include "base64.h"
#include "compression.h"
#include "encryption.h"
#include "network.h"
#include "structures.h"

#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void execute_call(uint8_t n_calls, uint8_t* calls) {
	int server_socket;
	struct sockaddr_in server_address;

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		perror("Failed to create socket");
		exit(1);
	}

	if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr)
        <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(1);
    }

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(1337);

	if (connect(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		perror("Failed to connect to the server");
		exit(1);
	}

	printf("Connected to the server...\n");

	User user;
	char username[] = "admin";
	strcpy(user.username, username);
	char pw[sizeof(user.password)] = "private";
	block_encrypt(pw, sizeof(user.password));
	memcpy(user.password, pw, sizeof(user.password));

	Session session;
	session.user = user;
	session.n_calls = n_calls;
	for (uint8_t i = 0; i < n_calls; i++) {
		session.calls[i] = (Call){calls[i*2], calls[i*2+1]};
	}

	size_t session_size = sizeof(Session);
	unsigned char* buffer = (unsigned char*)malloc(session_size);
	memcpy(buffer, &session, session_size);

	size_t compressed_length;
	unsigned char* compressed_buffer = compress(buffer, session_size, &compressed_length);
	free(buffer);

	size_t encoded_length;
	char* encoded_data = base64_encode(compressed_buffer, compressed_length, &encoded_length);
	free(compressed_buffer);

	if (send(server_socket, encoded_data, encoded_length + 1, 0) < 0) {
		perror("Failed to send message to the server");
		free(encoded_data);
		close(server_socket);
		exit(1);
	}
	free(encoded_data);

	char* buffer_rcv;
	int buffer_size;
	size_t length;
	int out = recv_to_null(server_socket, &buffer_rcv, &buffer_size, &length);
	if (out) {
		printf("Failed to receive data: err %d\n", out);
		close(server_socket);
		exit(1);
	}
	printf("buff=%s, length=%lu\n", buffer_rcv, length);

	size_t decoded_length;
	unsigned char* decoded_buffer = base64_decode(buffer_rcv, length, &decoded_length);
	free(buffer_rcv);

	size_t decompressed_length;
	unsigned char* decompressed_buffer = decompress(decoded_buffer, decoded_length, &decompressed_length);
	free(decoded_buffer);

	Response response;
	memcpy(&response, decompressed_buffer, sizeof(Response));
	free(decompressed_buffer);

	printf("Response: %s\n", response.data);

	close(server_socket);
}

int main() {
	uint8_t calls[] = {PUSH, 'f', PUSH, 'l', PUSH, 'a', PUSH, 'g', PUSH, '\0', LOAD, 0};
	execute_call(sizeof(calls) / sizeof(int8_t) / 2, calls);
}
