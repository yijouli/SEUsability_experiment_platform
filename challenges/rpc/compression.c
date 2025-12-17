#include <stdio.h>
#include <stdlib.h>

unsigned char* compress(const unsigned char* input, size_t length, size_t* compressed_length) {
	size_t allocated_size = 1;
	unsigned char* compressed = (unsigned char*)malloc(allocated_size);
	size_t count = 1;
	size_t j = 0;
	for (size_t i = 1; i <= length; i++) {
		if (i < length && input[i] == input[i - 1]) {
			count++;
		} else {
			if (j + 2 >= allocated_size) {
				allocated_size *= 2;
				compressed = (unsigned char*)realloc(compressed, allocated_size);
			}
			compressed[j++] = input[i - 1];
			while (count > 0) {
				if (j + 1 >= allocated_size) {
					allocated_size *= 2;
					compressed = (unsigned char*)realloc(compressed, allocated_size);
				}
				size_t chunk = count > 255 ? 255 : count;
				compressed[j++] = (unsigned char)chunk;
				count -= chunk;
			}
			count = 1;
		}
	}
	*compressed_length = j;
	return compressed;
}

unsigned char* decompress(const unsigned char* input, size_t compressed_length, size_t* decompressed_length) {
	size_t allocated_size = 1;
	unsigned char* decompressed = (unsigned char*)malloc(allocated_size);
	size_t j = 0;
	for (size_t i = 0; i < compressed_length; i++) {
		unsigned char ch = input[i];
		i++;
		if (i >= compressed_length) {
			return NULL;
		}
		unsigned char count = input[i];
		for (size_t k = 0; k < count; k++) {
			if (j >= allocated_size) {
				allocated_size *= 2;
				decompressed = (unsigned char*)realloc(decompressed, allocated_size);
			}
			decompressed[j++] = ch;
		}
	}
	*decompressed_length = j;
	return decompressed;
}
