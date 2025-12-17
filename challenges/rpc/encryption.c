#include "encryption.h"
#include <stdio.h>


void gen_key(uint32_t key[], int n) {
	srand(time(NULL));
	for (size_t i = 0; i < n; i++) {
		key[i] = rand();
	}
}

void round_key(uint32_t key[], int n) {
	for (size_t i = 0; i < n; i++) {
		key[i] += rand();
	}
}

void tea_encrypt(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) {
	uint32_t v0 = v[0], v1 = v[1], sum = 0, delta = 0x9E3779B9;
	for (size_t i = 0; i < num_rounds; i++) {
		v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
		sum += delta;
		v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
	}
	v[0] = v0;
	v[1] = v1;
}

void tea_decrypt(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) {
	uint32_t v0 = v[0], v1 = v[1], delta = 0x9E3779B9, sum = delta * num_rounds;
	for (size_t i = 0; i < num_rounds; i++) {
		v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
		sum -= delta;
		v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
	}
	v[0] = v0;
	v[1] = v1;
}

void block_encrypt(char* buff, int size) {
	uint32_t key[4];
	gen_key(key, sizeof(key)/sizeof(key[0]));
	for (size_t i = 0; i < size; i+=8) {
		round_key(key, sizeof(key)/sizeof(key[0]));
		tea_encrypt(32, (uint32_t*)&(buff[i]), key);
	}
}

void block_decrypt(char* buff, int size) {
	uint32_t key[4];
	gen_key(key, sizeof(key)/sizeof(key[0]));
	for (size_t i = 0; i< size; i+=8) {
		round_key(key, sizeof(key)/sizeof(key[0]));
		tea_decrypt(32, (uint32_t*)&(buff[i]), key);
	}
}
