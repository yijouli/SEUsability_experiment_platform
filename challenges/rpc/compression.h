#include <stdlib.h>

unsigned char* compress(const unsigned char* input, size_t length, size_t* compressed_length);
unsigned char* decompress(const unsigned char* input, size_t compressed_length, size_t* decompressed_length);
