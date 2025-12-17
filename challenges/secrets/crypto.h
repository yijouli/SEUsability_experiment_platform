#pragma once

#include <stdint.h>
#include <stdlib.h>

void encrypt(uint8_t* text[8], uint8_t const key[4]);
void decrypt(uint8_t* text[8], uint8_t const key[4]);
void block_encrypt(uint8_t** text, uint8_t const key[4], size_t* length);
void block_decrypt(uint8_t** text, uint8_t const key[4], size_t* length);
unsigned long djb2_hash(const unsigned char* str);