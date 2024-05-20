#include "cli_base.h"

#include <stdio.h>
#include <string.h>

void tok_cmd(char *cmd_in, char *tok_out) {
  char tmp[MAX_CMD_LEN + 1] = { 0 };
  const char delim[2] = " ";
  strip_ws(cmd_in);
  strncpy(tmp, cmd_in, sizeof(tmp) - 1);
  char *tmp_tok;
  // Linter rejects strtok and suggests strtok_r, but compiler gives implicit-function-declaration
  // error for strtok_r strtok and strtok_r perform the same functionality but strtok_r is
  // thread-safe
  tmp_tok = strtok(tmp, delim);  // NOLINT

  if (tmp_tok != NULL) {
    snprintf(tok_out, MAX_CMD_LEN + 1, "%s", tmp_tok);
    snprintf(cmd_in, MAX_CMD_LEN + 1, "%s", cmd_in + strlen(tmp_tok));
    strip_ws(cmd_in);
  } else {
    tok_out[0] = '\0';
  }
}

void strip_ws(char *str) {
  char *start = str;
  while (*start == ' ') {
    start++;
  }
  memmove(str, start, strlen(start) + 1);
}
