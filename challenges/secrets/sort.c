#include "sort.h"
#include <string.h>

int str_sort_partition(char** arr, int low, int high) {
    char* pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (strcmp(arr[j], pivot) < 0) {
            i++;
            char* temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    char* temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return (i + 1);
}

void str_sort(char** arr, int low, int high) {
    if (low < high) {
        int pi = str_sort_partition(arr, low, high);

        str_sort(arr, low, pi - 1);
        str_sort(arr, pi + 1, high);
    }
}
