#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

typedef int (*Cmd)(int, char *[]);

typedef struct CmdTable {
  char *cmdName;
  Cmd cmd;
} CmdTable;

int cmdPwd__Main(int, char *[]);

CmdTable builtinCmds[] = {
    { "pwd", cmdPwd__Main }
};

#endif /* ! SHELL_COMMANDS_H */
