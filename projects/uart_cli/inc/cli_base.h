#pragma once

#define MAX_CMD_LEN 50

typedef struct CmdStruct {
  const char *cmd_name;
  void (*cmd_func)(char *input);
} CmdStruct;

void tok_cmd(char *cmd_in, char *tok_out);
void strip_ws(char *str);
