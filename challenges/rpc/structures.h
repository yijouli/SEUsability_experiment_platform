#pragma once

#include <stdbool.h>

typedef enum {
	PUSH,
	POP,
	XOR,
	SORT,
	LOAD,
} Opcodes;

typedef struct {
	Opcodes opcode;
	char data;
} Call;

typedef struct {
	char username[16];
	char password[16];
} User;

typedef struct {
	User user;
	Call calls[8];
	uint8_t n_calls;
} Session;

typedef struct {
	bool state;
	char data[32];
} Response;
