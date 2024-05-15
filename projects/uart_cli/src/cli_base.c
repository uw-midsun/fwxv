#include "cli_base.h"

#include <stdio.h>
#include <string.h>

// Helper function to separate first argument from rest of string
void tok_cmd(char *cmd_in, char *tok_out) {
  char tmp[MAX_CMD_LEN + 1] = { 0 };
  const char delim[2] = " ";
  strncpy(tmp, cmd_in, sizeof(tmp));
  char *tmp_tok;
  tmp_tok = strtok(tmp, delim);  // NOLINT

  snprintf(tok_out, MAX_CMD_LEN + 1, "%s", tmp_tok);
  snprintf(cmd_in, MAX_CMD_LEN + 1, "%s", cmd_in + strlen(tmp_tok) + 1);
}

void strip_ws(char *str) {
  while (*str == ' ') {
    memmove(str, str + 1, sizeof(str) - sizeof(*str));
  }
}
