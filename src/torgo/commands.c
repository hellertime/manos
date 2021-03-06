#include <torgo/commands.h>

CmdTable builtinCmds[] = {
    { "cat", cmdCat__Main }
,   { "ls",  cmdLs__Main }
,   { "pwd", cmdPwd__Main }
,   { "echo", cmdEcho__Main }
,   { "date", cmdDate__Main }
,   { "toast", cmdToast__Main }
,   { "sh", torgo_main }
,   { "lsmem", cmdLsmem__Main }
,   { "ps", cmdPs__Main }
,   { "fg", cmdFg__Main }
,   { "fizzbuzz", cmdFizzbuzz__Main }
};
