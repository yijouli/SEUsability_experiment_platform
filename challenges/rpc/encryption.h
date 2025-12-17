#pragma once

#include <stdint.h>
#include <time.h>
#include <stdlib.h>

void gen_key(uint32_t key[], int n);
void tea_encrypt(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);
void tea_decrypt(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]);
void block_encrypt(char* buff, int size);
void block_decrypt(char* buff, int size);
