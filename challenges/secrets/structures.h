#pragma once

typedef int (*CommandFunc)(char** args);

typedef struct {
    char username[20];
    char password[20];
    char key[5];
} Session;

typedef struct {
    char name[20];
    CommandFunc func;
} CommandInfo;

typedef struct {
    char* command;
    char** args;
} Command;
