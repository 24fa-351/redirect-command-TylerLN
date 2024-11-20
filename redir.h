#ifndef REDIR_H
#define REDIR_H

#include <stddef.h>

void redirect(const char *input_file, const char *output_file);
void add_character_to_string(char *str, char c, size_t max_size);
void split(const char *cmd, char *words[], char delimiter);
int find_absolute_path(const char *cmd, char *absolute_path);

#endif
