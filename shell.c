#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>


// Function to read a line from stdin
char* readLine(void) {
    char* line = NULL;
    ssize_t bufsize = 0; // have getline allocate a buffer for us
    getline(&line, &bufsize, stdin);
    return line;
}

// Function to split the line into arguments
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char** splitLine(char* line) {
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char** tokens = malloc(bufsize * sizeof(char*));
    char* token;

    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

// Function to launch a program
int launch(char** args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("lsh");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

// Main loop of the shell
void loop(void) {
    char* line;
    char** args;
    int status;

    do {
        printf("> ");
        line = readLine();
        args = splitLine(line);
        status = launch(args);

        free(line);
        free(args);
    } while (status);
}

void sigintHandler(int sig_num) {
    // Reset handler to catch SIGINT next time
    signal(SIGINT, sigintHandler);
    printf("\nCannot be terminated using Ctrl+C\n");
    fflush(stdout);
}


// Main entry point of the program
int main(int argc, char** argv) {
    // Load config files, if any.

     // Setup the signal handler
    signal(SIGINT, sigintHandler);

    // Run command loop.
    loop();

    // Perform any shutdown/cleanup.

    return EXIT_SUCCESS;
}
