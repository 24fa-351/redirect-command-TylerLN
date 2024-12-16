#include "redir.h"
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 1000
#endif

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <input_file> <command> <output_file>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *input_file  = argv[1];
    const char *output_file = argv[argc - 1];
    const char *command     = argv[2];

    int fds[2];
    if (pipe(fds) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(fds[1]);
        dup2(fds[0], STDIN_FILENO);
        close(fds[0]);

        redirect(NULL, output_file);

        char *cmd[1000];
        split(command, cmd, ' ');

        char abs_path[PATH_MAX];
        if (find_absolute_path(cmd[0], abs_path)) {
            free(cmd[0]);
            cmd[0] = strdup(abs_path);
        } else {
            fprintf(stderr, "Command not found: %s\n", cmd[0]);
            exit(EXIT_FAILURE);
        }

        if (execv(cmd[0], cmd) < 0) {
            perror("execv failed");
            for (int i = 0; cmd[i] != NULL; i++)
                free(cmd[i]);
            exit(EXIT_FAILURE);
        }
    } else {
        close(fds[0]);

        int fdin;
        if (strcmp(input_file, "-") == 0) {
            fdin = STDIN_FILENO;
        } else {
            fdin = open(input_file, O_RDONLY);
            if (fdin < 0) {
                perror("Failed to open input file");
                exit(EXIT_FAILURE);
            }
        }

        char buffer[256];
        ssize_t bytes_read;
        while ((bytes_read = read(fdin, buffer, sizeof(buffer))) > 0)
            write(fds[1], buffer, bytes_read);

        if (fdin != STDIN_FILENO)
            close(fdin);
        close(fds[1]);
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
}
