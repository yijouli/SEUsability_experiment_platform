#include "base64.h"
#include "compression.h"
#include "encryption.h"
#include "network.h"
#include "network.h"
#include "sort.h"
#include "structures.h"

#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool is_admin(User* user) {
	char* decrypted_password = malloc(sizeof(user->password));
	memcpy(decrypted_password, user->password, sizeof(user->password));
	block_decrypt(decrypted_password, sizeof(decrypted_password));

	bool is_admin = true;

	is_admin &= (strcmp(user->username, "admin") == 0);
	is_admin &= (strcmp(decrypted_password, "private") == 0);

   return is_admin;
}

int load(uint8_t* data, uint8_t* size, uint8_t max) {
	char tmp[*size];
	strncpy(tmp, (char*)data, *size);
	FILE* file = fopen(tmp, "r");
	if (file == NULL) {
		return 1;
	}
	*size = fread((char*)data, sizeof(char), max, file);
	fclose(file);
	return 0;
}

void xor(uint8_t* buff, uint8_t val, uint8_t size) {
	for (size_t i = 0; i < size; i++) {
		buff[i] ^= val;
	}
}

int min(int x, int y) {
	return (((x) <= (y)) ? (x) : (y));
}

void dispatch(Session* session, Response* response) {
	uint8_t buff[32] = {0};
	uint8_t j = 0;
	uint8_t calls = min(session->n_calls, sizeof(session->calls)/sizeof(session->calls[0]));
	bool err = false;
	for (size_t i = 0; i < calls && !err; i++) {
		Call call = session->calls[i];
		switch (call.opcode) {
			case PUSH:
				if (j >= sizeof(buff)) {
					err = true;
					break;
				}
				buff[j] = call.data;
				j++;
				break;
			case POP:
				if (j-1 < 0) {
					err = true;
					break;
				}
				j--;
				break;
			case XOR:
				xor(buff, call.data, j);
				break;
			case LOAD:
				if (is_admin(&session->user)) {
					err = load(buff, &j, sizeof(buff));
				}
				break;
			case SORT:
				sort((uint8_t*)buff, j);
				break;
			default:
				err = true;
				break;
		}
	}

	response->state = err;
	memcpy(response->data, buff, sizeof(response->data));
}

void decode_request(Session* session, char* buffer, int length) {
	size_t decoded_length;
	unsigned char* decoded_buffer = base64_decode(buffer, length, &decoded_length);

	size_t decompressed_length;
	unsigned char* decompressed_buffer = decompress(decoded_buffer, decoded_length, &decompressed_length);

	memcpy(session, decompressed_buffer, sizeof(Session));

	free(decoded_buffer);
	free(decompressed_buffer);
}

void encode_response(Response* response, char** buffer, size_t* length) {
	size_t response_size = sizeof(Response);
	unsigned char* tmp_buffer = (unsigned char*)malloc(response_size);
	memcpy(tmp_buffer, response, response_size);

	size_t compressed_length;
	unsigned char* compressed_buffer = compress(tmp_buffer, response_size, &compressed_length);
	free(tmp_buffer);

	*buffer = base64_encode(compressed_buffer, compressed_length, length);
	free(compressed_buffer);
}

int recv_session(int client_socket, Session* session) {
	char* buffer;
	int buffer_size;
	size_t length;

	int out = recv_to_null(client_socket, &buffer, &buffer_size, &length);
	if (out) return out;

	decode_request(session, buffer, length);
	free(buffer);

	return 0;
}

void send_response(int client_socket, Response* response) {
	char* buffer;
	size_t length;
	encode_response(response, &buffer, &length);

	if (send(client_socket, buffer, length+1, 0) < 0) {
		// perror("Failed to send message to the server");
		exit(EXIT_FAILURE);
	}

	// fprintf(stdout, "Sent response data: %s\n", buffer);

	free(buffer);
}

int handle_request(int client_socket) {
	// fprintf(stdout, "Client connected\n");

	Session session;
	recv_session(client_socket, &session);
	Response response;
	dispatch(&session, &response);
	send_response(client_socket, &response);

	return 0;
}

int request_loop() {
	int server_socket, client_socket;
	struct sockaddr_in server_address;

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		// perror("Failed to create socket");
		exit(EXIT_FAILURE);
	}

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(1337);
	server_address.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		// perror("Failed to bind socket to address");
		exit(EXIT_FAILURE);
	}

	if (listen(server_socket, 5) < 0) {
		// perror("Failed to listen on socket");
		exit(EXIT_FAILURE);
	}

	while(1) {
		client_socket = accept(server_socket, NULL, NULL);
		if (client_socket < 0) {
			// perror("Failed to accept connection");
			continue;
		}

		pid_t pid = fork();
		if (pid < 0) {
			// perror("Failed to fork");
			continue;
		}

		if (pid == 0) {
			close(server_socket);
			// fprintf(stdout, "Handling client\n");
			handle_request(client_socket);
			// fprintf(stdout, "Client handled\n");
			close(client_socket);
			exit(EXIT_SUCCESS);
		} else {
			close(client_socket);
		}
	}

	close(server_socket);
}

int main() {
	request_loop();
	return 0;
}
