#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define DEBUG_FILE "debug.txt"

int main(int argc, char* argv[]) {
    if (argc != 6) {
        printf("Insufficient arguments.\n");
        return 0;
    }
    char* path = argv[1]; /* path to the reader directory. */
    char* debugFN = (char*)calloc(strlen(path) + strlen(DEBUG_FILE) + 1, sizeof(char));
    strcpy(debugFN, path);
    strcat(debugFN, DEBUG_FILE);
    FILE* fp = NULL;
    if (strcmp(argv[5], "true") == 0) { /* yes, I want debug info. */
        fp = fopen(debugFN, "a");
        int i = 0;
        if (fp == NULL) {
            printf("NULL file pointer. Error: %d.\n", errno);
            return 0;
        }
        for (; i < argc; ++i) {
            fputs(argv[i], fp);
            fputs("\n", fp);
        }
    }
	int result;
    /* ret script info: path + name. */
	char* scriptInfo = (char*)calloc(strlen(path) + strlen(argv[2]) + 1, sizeof(char));
    strcpy(scriptInfo, argv[1]); /* copy path. */
    strcat(scriptInfo, argv[2]); /* copy script name. */
	
    if (strcmp(argv[5], "true") == 0) {
        fputs("path: ", fp); fputs(argv[1], fp); fputs("\n", fp);
        fputs("script name: ", fp); fputs(argv[2], fp); fputs("\n", fp);
        fputs("device name: ", fp); fputs(argv[3], fp); fputs("\n", fp);
        fputs("major: ", fp); fputs(argv[4], fp); fputs("\n", fp);
    }
	char* env[] = {
		"HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin:",
		NULL
    };
    fclose(fp);
    /* fork. */
    int pid = fork();
    if (pid == 0) { /* child proc. */
        execle(scriptInfo, scriptInfo, argv[1], argv[3], argv[4], (char*)NULL, env);
    }
    /* release memory. */
    free(debugFN);
    free(scriptInfo);
	return 0;
}
/**
 * ./caller.o /home/licenta/Documents/LICENTA/keylogger/test/reader/ getPSOutput.sh ps_reader 243 true
 * 
 * ./getPSOutput.sh /home/licenta/Documents/LICENTA/keylogger/test/reader/ ps_reader 243
*/
