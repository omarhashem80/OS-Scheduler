#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define FILENAME "file.txt"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <algorithm> <path> <time_slice>\n", argv[0]);
        return 1;
    }

   

    // Parse arguments
    int algoNo = atoi(argv[1]);
    const char *path = argv[2];
    int timeSlice = atoi(argv[3]);

    FILE *file = fopen(FILENAME, "w");
    if (file == NULL) {
        perror("Error opening file");
        return 0;
    }
    sleep(10);
    // Write data to the file
    char data[] = "1";
    fprintf(file, "%s", data);

    // Close the file
    fclose(file);
    printf("end\n");
    return 0;
}
