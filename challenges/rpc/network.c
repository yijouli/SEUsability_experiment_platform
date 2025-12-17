#include "network.h"

int recv_to_null (int client_socket, char** buffer, int* buff_size, size_t* length) {
	*buffer = malloc(1);
	*buff_size = 1;
	*length = 0;
	while (1) {
		char c;
		if (recv(client_socket, &c, 1, 0) < 0) {
			// perror("Failed to receive data");
			return 1;
		}
		if (c == '\0') {
			break;
		}
		if (*length == *buff_size) {
			(*buff_size) *= 2;
			*buffer = realloc(*buffer, *buff_size);
			if (*buffer == NULL) {
				// perror("Failed to resize buffer");
				return 1;
			}
		}
		(*buffer)[*length] = c;
		(*length)++;
	}
	(*buffer)[*length] = '\0';
	return 0;
}
