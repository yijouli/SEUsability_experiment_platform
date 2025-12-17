#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <regex.h>

#include "crypto.h"
#include "sort.h"
#include "structures.h"

int help_cmd(char** args);
int register_cmd(char** args);
int login_cmd(char** args);
int logout_cmd(char** args);
int ls_cmd(char** args);
int new_cmd(char** args);
int show_cmd(char** args);
int exit_cmd(char** args);

CommandInfo commandInfo[] = {
	{"register", register_cmd},
	{"login", login_cmd},
	{"logout", logout_cmd},
	{"ls", ls_cmd},
	{"new", new_cmd},
	{"show", show_cmd},
	{"exit", exit_cmd},
};

Session* session = NULL;

void derive_key(const char* password, uint8_t key[5]) {
	uint8_t a = 101;
	uint8_t b = 251;
	const size_t str_len = strlen(password);
	for (size_t i = 0; i < str_len; i++) {
		if (i%2) {
			a = (a ^ password[i]) << 1 | (a ^ password[i]) >> 7;
		} else {
			b = (b ^ password[i]) << 1 | (b ^ password[i]) >> 7;
		}
	}
	key[0] = a;
	key[1] = b;
	key[2] = ~a;
	key[3] = ~b;
	key[4] = '\0';
	/*
	uint32_t result = 0;
	for (int i = 0; i < 4; i++) {
		result |= ((unsigned long)key[i] << (8 * i));
	}
	return result;
	*/
}

unsigned long get_pwd_hash(const char* password) {
	uint8_t key[5] = {'\0'};
	derive_key(password, key);
	return djb2_hash(key);
}

int save_hash(char* encoded_username, char* password) {
	int ret;
	char dot_shadow[] = "/.shadow";
	char* hashfile_path = calloc(strlen(encoded_username) + strlen(dot_shadow) + 1, sizeof(char));
	sprintf(hashfile_path, "%s%s", encoded_username, dot_shadow);

	FILE* file = fopen(hashfile_path, "wb");
	if (file == NULL) {
		ret = 1;
	} else {
		fprintf(file, "%lu", get_pwd_hash(password));
		ret = 0;
	}
	fclose(file);
	free(hashfile_path);
	return ret;
}

bool str_has_nonprint_ascii(const char* password) {
	for (int i = 0; password[i] != '\0'; i++) {
		if (password[i] < 33 || password[i] > 126) return true;
	}
	return false;
}

int register_cmd(char** args) {
	if (session != NULL || args[0] == NULL || args[1] == NULL) {
		return 1;
	}
	if (str_has_nonprint_ascii(args[0])) return 1;
	if (str_has_nonprint_ascii(args[1])) return 1;
	if (access(args[0], F_OK) != -1) {
		return 1;
	}
	if (mkdir(args[0], 0755) == -1) {
		return 1;
	}
	if (save_hash(args[0], args[1]) == 1) {
		return 1;
	}
	return 0;
}

bool is_password_valid(char* password) {
	int ret;
	FILE* file = fopen(".shadow", "rb");
	if (file == NULL) {
		ret = false;
	} else {
		unsigned long hash = get_pwd_hash(password);
		unsigned long shadow_hash;
		int err = fscanf(file, "%lu", &shadow_hash);
		if (err == EOF) {
			exit(1);
		}
		ret = shadow_hash == hash;
	}
	fclose(file);
	return ret;
}

int login_cmd(char** args) {
	if (session != NULL || args[0] == NULL || args[1] == NULL) return 1;
	if (str_has_nonprint_ascii(args[1])) return 1;
	if (chdir(args[0])) {
		return 1;
	}
	if (!is_password_valid(args[1])) {
		if (chdir("..")) {
			exit(1);
		}
		return 1;
	}
	session = malloc(sizeof(Session));
	strcpy(session->username, args[0]);
	strcpy(session->password, args[1]);
	derive_key(session->password, session->key);
	return 0;
}

int logout_cmd(char** args) {
	if (session == NULL) return 1;
	free(session);
	session = NULL;
	if (chdir("..")) exit(1);
	return 0;
}

int ls_cmd(char** args) {
	if (session == NULL) return 1;

	bool hidden = false;
	char* path = NULL;

	for (size_t i = 0; args[i] != NULL; i++) {
		if (strcmp(args[i], "-a") == 0) {
			hidden = true;
		} else {
			path = args[i];
		}
	}

	if (path == NULL) path = ".";

	DIR* dir = opendir(path);
	if (dir == NULL) return 1;

	struct dirent* entry;
	unsigned char** paths = NULL;
	size_t num_paths = 0;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.') continue;

		char* full_path;
		if (asprintf(&full_path, "%s/%s", path, entry->d_name) < 0) {
 			return 1;
		}
		struct stat path_stat;
		stat(full_path, &path_stat);
		bool is_dir = false;
		if (S_ISDIR(path_stat.st_mode)) {
			is_dir = true;
		}

		path = strdup(entry->d_name);
		size_t path_len = strlen(path);

		if (path == NULL) {
			closedir(dir);
			return 1;
		}

		if (is_dir) {
			path = realloc(path, path_len + 2);
			path[path_len] = '/';
			path_len++;
		} else {
			path = realloc(path, path_len + 1);
		}
		path[path_len] = '\0';

		paths = realloc(paths, (num_paths + 1) * sizeof(char*));
		paths[num_paths++] = path;

		free(full_path);
	}
	closedir(dir);

	str_sort((char**)paths, 0, num_paths - 1);
	for (size_t i = 0; i < num_paths; i++) {
		printf("%s\n", paths[i]);
		free(paths[i]);
	}

	free(paths);
	return 0;
}

int new_cmd(char** args) {
	if (session == NULL) return 1;

	bool force = false;
	char* path = NULL;

	for (size_t i = 0; args[i] != NULL; i++) {
		if (strcmp(args[i], "-f") == 0) {
			force = true;
		} else {
			path = args[i];
		}
	}

	if (path == NULL) {
		return 1;
	}

	char* content = NULL;
	size_t content_len = 0;

	if (!force && access(path, F_OK) != -1) {
		return 1;
	}

	FILE* file = fopen(path, "w");
	if (file == NULL) {
		return 1;
	}

	if (getline(&content, &content_len, stdin) == -1) {
		fclose(file);
		return 1;
	}

	size_t length = strlen(content)+1;
	uint8_t* encrypted_content = (uint8_t*)malloc(length);
	memcpy(encrypted_content, content, length);

	block_encrypt(&encrypted_content, (uint8_t*)session->key, &length);

	fwrite("ENC!", sizeof(char), 4, file);
	fwrite(encrypted_content, sizeof(char), length, file);
	fclose(file);

	free(encrypted_content);
	return 0;
}

bool is_valid_path(char* str) {
	regex_t regex;
	int ret = regcomp(&regex, "^[a-zA-Z0-9/\\.]+$", REG_EXTENDED);
	if (ret) {
		exit(1);
	}
	ret = regexec(&regex, str, 0, NULL, 0);
	regfree(&regex);
	return !ret;
}

int show_cmd(char** args) {
	if (session == NULL || args[0] == NULL || !is_valid_path(args[0])) return 1;

	struct stat path_stat;
	stat(args[0], &path_stat);
	if (S_ISDIR(path_stat.st_mode)) {
		return 1;
	}

	FILE* file = fopen(args[0], "r");
	if (file == NULL) {
		return 1;
	}

	char buffer[1024];
	char* file_content = malloc(1);
	file_content[0] = '\0';
	size_t total_bytes_read = 0;
	size_t bytes_read;
	while ((bytes_read = fread(buffer, sizeof(char), sizeof(buffer) - 1, file)) > 0) {
		buffer[bytes_read] = '\0';
		total_bytes_read += bytes_read;
		file_content = realloc(file_content, total_bytes_read + 1);
		strcat(file_content, buffer);
	}
	fclose(file);

	if (strncmp(file_content, "ENC!", 4) == 0) {
		size_t size_file_content_nomagic = total_bytes_read-4;
		char* file_content_nomagic = malloc(size_file_content_nomagic);
		memcpy(file_content_nomagic, file_content+4, size_file_content_nomagic);
		block_decrypt((uint8_t**)&file_content_nomagic, (uint8_t*)session->key, &size_file_content_nomagic);
		printf("%s", file_content_nomagic);
		free(file_content_nomagic);
    } else {
	    printf("%s", file_content);
    }
	free(file_content);
	return 0;
}

int exit_cmd(char** args) {
	if (args[0] != NULL) exit(atoi(args[0]));
	exit(0);
}

int handle_command(Command* command) {
	if (command->command == NULL) {
		return 0;
	}
	char* cmd = command->command;
	char** args = command->args;
	for (size_t j = 0; j < sizeof(commandInfo) / sizeof(CommandInfo); j++) {
		if (strcmp(cmd, commandInfo[j].name) == 0) {
			return commandInfo[j].func(args);
		}
	}
	return 1;
}

char* strtrim(char* str) {
	while (isspace((unsigned char)*str)) str++;
	if (*str == 0) return str;

	char* end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end)) end--;
	end[1] = '\0';

	return str;
}

int parse_command(char* line, Command** command_ptr) {
	Command* parsed_command = malloc(sizeof(Command));

	parsed_command->command = strtok(line, " \t\r\n");
	char** args = malloc(sizeof(char*));
	if (args == NULL) return 1;

	char* arg = strtok(NULL, " \t\r\n");
	int num_args = 0;
	while (arg != NULL) {
		args[num_args] = strdup(arg);
		num_args++;
		args = realloc(args, (num_args + 1) * sizeof(char*));
		if (args == NULL) return 1;
		arg = strtok(NULL, " \t\r\n");
	}
	args[num_args] = NULL;
	parsed_command->args = args;

	*command_ptr = parsed_command;

	return 0;
}

void free_command(Command* command) {
	for (size_t j = 0; command->args[j] != NULL; j++) {
		free(command->args[j]);
	}
	free(command->args);
	free(command);
}

uint32_t combineKey(uint8_t key[4]) {
	uint32_t result = 0;
	for (int i = 0; i < 4; i++) {
		result |= ((unsigned long)key[i] << (8 * i));
	}
	return result;
}

void printBits(uint32_t num) {
	for (int i = 31; i >= 0; i--) {
		printf("%u", (num >> i) & 1);
		if (i % 8 == 0) {
			printf(" "); // Add a space every 8 bits for readability
		}
	}
	printf("\n");
}


int main() {
	const char wd[] = "sec";
	mkdir(wd, 0755);
	if (chdir(wd)) exit(1);

	int status = 0;

	while (1) {
		char* line = NULL;
		size_t line_len = 0;

		if (status == 0) printf("[ ]> ");
		else printf("[!] > ");

		if (getline(&line, &line_len, stdin) == -1) {
			if (feof(stdin)) { // Ctrl+D
				break;
			}
			continue;
		}

		Command* command;
		status = parse_command(line, &command);
		if (!status) {
			status = handle_command(command);
		}
		free_command(command);
		free(line);
	}
	return 0;
}
