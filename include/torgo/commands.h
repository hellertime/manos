#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

typedef int (*Cmd)(int, char * const []);

typedef struct CmdTable {
  char *cmdName;
  Cmd cmd;
} CmdTable;

int cmdLs__Main(int, char * const []);
int cmdPwd__Main(int, char * const []);

extern CmdTable builtinCmds[2];
#endif /* ! SHELL_COMMANDS_H */
