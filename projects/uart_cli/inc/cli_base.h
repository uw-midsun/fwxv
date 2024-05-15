#ifndef PROJECTS_UART_CLI_INC_CLI_BASE_H_
#define PROJECTS_UART_CLI_INC_CLI_BASE_H_

#define _POSIX_C_SOURCE 200112L
#define MAX_CMD_LEN 50

typedef struct CmdStruct {
  const char *cmd_name;
  void (*cmd_func)(char *input);
} CmdStruct;

void tok_cmd(char *cmd_in, char *tok_out);
void strip_ws(char *str);
#endif  // PROJECTS_UART_CLI_INC_CLI_BASE_H_
