#include <stdio.h>
#include <string.h>

int main() {
    char input[50];  // Buffer to store user input
    char password[] = "th1s_1s_my_s3cr3t_p4ssw0rd";  // Secret password
    char flag[100];  // Buffer to store the flag

    // Open the file containing the flag
    FILE *flagFile = fopen("/flag", "r");
    if (flagFile == NULL) {
        printf("Error: Could not open the flag file.\n");
        return 1;
    }

    // Read the flag from the file
    if (fgets(flag, sizeof(flag), flagFile) == NULL) {
        printf("Error: Could not read the flag.\n");
        fclose(flagFile);
        return 1;
    }

    // Close the flag file
    fclose(flagFile);

    printf("Enter the secret password: ");
    fgets(input, sizeof(input), stdin);

    // Remove the newline character from fgets input
    input[strcspn(input, "\n")] = '\0';

    if (strcmp(input, password) == 0) {
        printf("Correct! Here is your flag: %s\n", flag);
    } else {
        printf("Incorrect password!\n");
    }

    return 0;
}

