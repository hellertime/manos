#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

typedef int (*Cmd)(int, char *[]);

struct CmdTable {
  char *cmdName;
  Cmd cmd;
};

extern struct CmdTable builtinCmds[];
extern int numBuiltinCmds;

#endif /* ! SHELL_COMMANDS_H */
