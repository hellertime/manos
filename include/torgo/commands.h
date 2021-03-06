#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

typedef int (*Cmd)(int, char * const []);

typedef struct CmdTable {
  char *cmdName;
  Cmd cmd;
} CmdTable;

int torgo_main(int, char * const []);
int cmdCat__Main(int, char * const []);
int cmdLs__Main(int, char * const []);
int cmdPwd__Main(int, char * const []);
int cmdEcho__Main(int, char * const []);
int cmdDate__Main(int, char * const []);
int cmdToast__Main(int, char * const []);
int cmdLsmem__Main(int, char * const[]);
int cmdPs__Main(int, char * const[]);
int cmdFg__Main(int, char * const[]);
int cmdFizzbuzz__Main(int, char * const[]);

extern CmdTable builtinCmds[11];
#endif /* ! SHELL_COMMANDS_H */
