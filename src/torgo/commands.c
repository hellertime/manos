#include <torgo/commands.h>

CmdTable builtinCmds[] = {
    { "cat", cmdCat__Main }
,   { "ls",  cmdLs__Main }
,   { "pwd", cmdPwd__Main }
,   { "echo", cmdEcho__Main }
,   { "date", cmdDate__Main }
};
