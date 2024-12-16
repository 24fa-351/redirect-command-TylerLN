#include "redir.h"
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 1000
#endif

void redirect(const char *input_file, const char *output_file) {
    if (input_file && strcmp(input_file, "-") != 0) {
        int fdin = open(input_file, O_RDONLY);
        if (fdin < 0) {
            perror("Failed to open input file");
            exit(EXIT_FAILURE);
        }
        dup2(fdin, STDIN_FILENO);
        close(fdin);
    }

    if (output_file && strcmp(output_file, "-") != 0) {
        int fdout = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fdout < 0) {
            perror("Failed to open output file");
            exit(EXIT_FAILURE);
        }
        dup2(fdout, STDOUT_FILENO);
        close(fdout);
    }
}

void add_character_to_string(char *str, char c, size_t max_size) {
    int len      = strlen(str);
    str[len]     = c;
    str[len + 1] = '\0';
}

void split(const char *cmd, char *words[], char delimiter) {
    int word_count           = 0;
    char current_word[10000] = "";
    const char *next_char    = cmd;

    while (*next_char != '\0') {
        if (*next_char == delimiter) {
            if (strlen(current_word) > 0) {
                words[word_count++] = strdup(current_word);
                current_word[0]     = '\0';
            }
        } else {
            add_character_to_string(current_word, *next_char,
                                    sizeof(current_word));
        }
        ++next_char;
    }

    if (strlen(current_word) > 0)
        words[word_count++] = strdup(current_word);
    words[word_count] = NULL;
}

int find_absolute_path(const char *cmd, char *absolute_path) {
    char *path = getenv("PATH");
    if (!path) {
        fprintf(stderr, "No PATH environment variable found.\n");
        strcpy(absolute_path, cmd);
        return 0;
    }

    size_t len = strlen(path);
    char temp_path[PATH_MAX];
    size_t start = 0;

    for (size_t ix = 0; ix <= len; ++ix) {
        if (path[ix] == ':' || path[ix] == '\0') {
            snprintf(temp_path, sizeof(temp_path), "%.*s/%s", (int)(ix - start),
                     path + start, cmd);
            if (access(temp_path, X_OK) == 0) {
                strcpy(absolute_path, temp_path);
                return 1;
            }
            start = ix + 1;
        }
    }

    strcpy(absolute_path, cmd);
    return 0;
}
