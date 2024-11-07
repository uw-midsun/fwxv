#pragma once

#define MAX_CMD_LEN 50

typedef struct CmdStruct {
  const char *cmd_name;
  void (*cmd_func)(char *input);
} CmdStruct;

// Separates first token from rest of string
// Token stored in tok_out, remaining string stored in cmd_in
void tok_cmd(char *cmd_in, char *tok_out);

// Strips whitespace before a string
void strip_ws(char *str);
