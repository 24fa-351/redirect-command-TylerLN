#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

void Redirect(const char *input_file, const char *output_file) {
  if (input_file) {
    int fdin = open(input_file, O_RDONLY);
    if (fdin < 0) {
      perror("Failed to open input file");
      exit(EXIT_FAILURE);
    }
    dup2(fdin, STDIN_FILENO);
    close(fdin);
  }

  if (output_file) {
    int fdout = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fdout < 0) {
      perror("Failed to open output file");
      exit(EXIT_FAILURE);
    }
    dup2(fdout, STDOUT_FILENO);
    close(fdout);
  }
}

char *find_absolute_path(const char *cmd) {
  char *path = getenv("PATH");
  if (!path) return NULL;

  char *token = strtok(path, ":");
  char absPath[PATH_MAX];

  while (token) {
    snprintf(absPath, sizeof(absPath), "%s/%s", token, cmd);
    if (access(absPath, X_OK) == 0) {
      return strdup(absPath);
    }
    token = strtok(NULL, ":");
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    fprintf(stderr, "Usage: %s <input_file> <command> <output_file>\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *input_file = argv[1];
  const char *output_file = argv[argc - 1];

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
    dup2(fds[0], STDIN_FILENO);
    close(fds[0]);
    close(fds[1]);

    Redirect(input_file, output_file);

    char **cmd = malloc((argc - 2) * sizeof(char *));
    char *command = argv[2];
    char *token = strtok(command, " ");
    int cmd_ix = 0;

    while (token) {
      cmd[cmd_ix++] = token;
      token = strtok(NULL, " ");
    }
    cmd[cmd_ix] = NULL;

    char *abs_path = find_absolute_path(cmd[0]);
    if (abs_path) {
      cmd[0] = abs_path;
    } else {
      fprintf(stderr, "Command not found: %s\n", cmd[0]);
      free(cmd);
      exit(EXIT_FAILURE);
    }

    if (execv(cmd[0], cmd) < 0) {
      perror("execv failed");
      free(cmd);
      free(abs_path);
      exit(EXIT_FAILURE);
    }
    free(cmd);
    free(abs_path);
  }

  close(fds[0]);

  int fdin = open(input_file, O_RDONLY);
  if (fdin < 0) {
    perror("Failed to open input file");
    exit(EXIT_FAILURE);
  }

  char buffer[256];
  ssize_t bytes_read;
  while ((bytes_read = read(fdin, buffer, sizeof(buffer))) > 0) {
    write(fds[1], buffer, bytes_read);
  }

  close(fdin);
  close(fds[1]);
  int status;
  pid_t wpid = waitpid(pid, &status, 0);
  return (wpid == pid && WIFEXITED(status)) ? WEXITSTATUS(status) : -1;
}
