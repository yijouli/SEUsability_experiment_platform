#pragma once

#include <stdlib.h>
#include <netinet/in.h>

int recv_to_null (int client_socket, char** buffer, int* buff_size, size_t* length);
