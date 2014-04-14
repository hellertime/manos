#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

typedef int (*Cmd)(int, char * const []);

typedef struct CmdTable {
  char *cmdName;
  Cmd cmd;
} CmdTable;

int cmdCat__Main(int, char * const []);
int cmdLs__Main(int, char * const []);
int cmdPwd__Main(int, char * const []);
int cmdEcho__Main(int, char * const []);
int cmdDate__Main(int, char * const []);

extern CmdTable builtinCmds[5];
#endif /* ! SHELL_COMMANDS_H */
