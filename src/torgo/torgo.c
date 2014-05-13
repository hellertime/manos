/**
 * shell.c
 *
 * Provides the command evaluation and interactive bits of the shell.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <manos.h>

#include <torgo/charbuf.h>
#include <torgo/commands.h>
#include <torgo/env.h>
#include <torgo/parser.h>

typedef enum {
  ShellStateRun,
  ShellStateEOF,
  ShellStateExit
} ShellState;

typedef struct Shell {
  Env*       env;
  Parser*    parser;
  CharBuf*   readBuf;
  ShellState state;
} Shell;

void freeShell(Shell *shell);

Shell* mkShell(void) {
  Shell *shell = kmalloc(sizeof *shell);
  if (! shell) goto exit;

  shell->env = NULL;
  shell->parser = NULL;
  shell->readBuf = NULL;
  shell->state = ShellStateRun;

  shell->env = mkEnv();
  if (! shell->env) goto fail;

  shell->parser = mkParser();
  if (! shell->parser) goto fail;

  shell->readBuf = mkCharBuf(32);
  if (! shell->readBuf) goto fail;

exit:
  return shell;

fail:
  freeShell(shell);
  shell = NULL;
  goto exit;
}

void freeShell(Shell *shell) {
  freeCharBuf(shell->readBuf);
  freeEnv(shell->env);
  freeParser(shell->parser);
  free(shell);
}

/*
 * readPromptShell :: Shell -> CStr -> Int -> CharBuf
 *
 * Fills a CharBuf with input until EOF or an unescaped
 * newline is entered, or the readMax is met (set to -1 to be unlimited)
 */
const CharBuf* readPromptShell(Shell *shell, const char *promptStr, int readMax) {
  int keepReading = 1;

  clearCharBuf(shell->readBuf);

  fputstr(rp->tty, promptStr);

  if (promptStr[strlen(promptStr) - 1] != ' ')
    fputchar(rp->tty, ' ');
  
  while (keepReading && (readMax == -1 || readMax > 0)) {
    char c;
    if (kread(rp->tty, &c, 1) == 0)
        c = 0; /* EOF */

    switch (c) {
      case 0:
        keepReading = 0;
        break;
      case 127: /* DEL */
          if (dropLastCharBuf(shell->readBuf) != 0)
              fputstr(rp->tty, "\b \b"); /* backup, erase, backup */
          else
              fputchar(rp->tty, '\a'); /* BEEP */
          break;
      case '\r':
      case '\n':
        keepReading = 0;
        if (c == '\r') {
            fputchar(rp->tty, c);
            c = '\n';
        }
        /* fall through */
      default:
        fputchar(rp->tty, c);
        appendCharBuf(shell->readBuf, c);
        break;
    }
  }

  return shell->readBuf;
}

/*
 * populateCmdArgsShell :: Env -> ParseResult -> IntPtr [Cstr]Ptr -> ()
 *
 * Allocates the argv vector, and populates it from tje ParseResult.
 * Sets up 'argc' and expands variables in the process of populating
 * the vector.
 */
int populateCmdArgsShell(Env *env, ParseResult *result, int *argc, char ***argv) {
  int argc_ = getLengthParseResult(result);
  char **argv_ = kmalloc((1 + argc_) * sizeof *argv_); /* sysexecv must have a NULL terminated array of pointers */

  ParseTokenIterator *tokens = getParseTokenIteratorParseResult(result);
  CharBuf *tokenBuilder = mkCharBuf(32);
  CharBuf *varBuilder = mkCharBuf(32);

  int bg = 0;

  for (int i = 0; i < argc_; i++) {
    const String *token = getNextParseTokenIterator(tokens);
    const char *tokenStr = fromString(token);

    clearCharBuf(tokenBuilder);

    while (*tokenStr) {
      switch (*tokenStr) {
        case '$':
          clearCharBuf(varBuilder);
          tokenStr++;
          if (*tokenStr == '_' || (*tokenStr >= 'a' && *tokenStr <= 'z') || (*tokenStr >= 'A' && *tokenStr <= 'Z')) {
            while (*tokenStr == '_' || (*tokenStr >= 'a' && *tokenStr <= 'z') || (*tokenStr >= 'A' && *tokenStr <= 'Z') || (*tokenStr >= '0' && *tokenStr <= '9')) {
              appendCharBuf(varBuilder, *tokenStr);
              tokenStr++;
            }

            String varStr;
            assignString(&varStr, fromCharBuf(varBuilder));
            const String* value = lookupVarEnv(env, &varStr);

            if (value)
              concatCharBuf(tokenBuilder, fromString(value));

          }
          break;
        default:
          appendCharBuf(tokenBuilder, *tokenStr);
          tokenStr++;
          break;
      }
    }
    
    const char* arg = fromCharBuf(tokenBuilder);
    argv_[i] = kmalloc(strlen(arg) + 1);
    memcpy(argv_[i], arg, strlen(arg));
  }

  if (argc_) {
     char* lastArg = argv_[argc_ - 1];
     if (*(lastArg + strlen(lastArg)) == '&') {
         *(lastArg + strlen(lastArg)) = 0;
        bg = 1;
     }
  }

  freeCharBuf(tokenBuilder);
  freeCharBuf(varBuilder);
  freeParseTokenIterator(tokens);
  *argc = argc_;
  *argv = argv_;
  return bg;
}

/*
 * main
 *
 * Main driver loop. Loads input from the prompt, and
 * drives the parse. Certain builtins that need to effect
 * the environment are defined here.
 */

#include "harvard.h"
#include "manos_ansi.h"

int torgo_main(int argc, char * const argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  const char *ps1 = "torgo > ", *ps2 = "> ";

  setSignalMask(SigStop, NULL);

  Shell *shell = mkShell();

  fputstr(rp->tty, "[2J[f");
  fputstr(rp->tty, harvard_ansi);
  fprintln(rp->tty, "");
  fputstr(rp->tty, manos_ansi);
  fputstr(rp->tty, "[G[m");
  fprintln(rp->tty, "");
  
  const char *ps = ps1;
  while (shell->state == ShellStateRun) {
    const CharBuf *input = readPromptShell(shell, ps, 32);

    if (isEmptyCharBuf(input)) {
      shell->state = ShellStateEOF;
      if (hasUnparsedInputParser(shell->parser)) {
        fputstr(rp->tty, "error: unexpected end-of-file\n");
      }
      fputstr(rp->tty, "\n");
      break;
    }

    addInputParser(shell->parser, fromCharBuf(input));

    while (hasUnparsedInputParser(shell->parser)) {
      ParseResult *result = parseInputParser(shell->parser);

      if (isCompleteParseResult(result)) {
        int cmdArgc;
        char **cmdArgv;
        int bg = populateCmdArgsShell(shell->env, result, &cmdArgc, &cmdArgv);
        int pid = kexec(cmdArgv[0], cmdArgv);

        if (bg)
            waitpid(pid);

        /*
         * Some commands need shell environment access and so cannot be passed of to 'exec' at the moment
         */
#if 0
        if (cmdArgc == 4 && strcmp(cmdArgv[0], "set") == 0 && strcmp(cmdArgv[2], "=") == 0) {
          String name;
          String value;

          assignString(&name, cmdArgv[1]);
          assignString(&value, cmdArgv[3]);
          addVarEnv(shell->env, &name, &value);
        } else if (cmdArgc == 2 && strcmp(cmdArgv[0], "unset") == 0) {
          String name;

          assignString(&name, cmdArgv[1]);
          unsetVarEnv(shell->env, &name);
        } else if (cmdArgc == 1 && strcmp(cmdArgv[0], "perror") == 0) {
          printf("Last status (%d): %s\n", shellErrno, strerror(shellErrno)); /* fromErr(shellErrno)); */
        } else {
          for (int i = 0; cmdArgc && i < numBuiltinCmds; i++) {
            if (strcmp(builtinCmds[i].cmdName, cmdArgv[0]) == 0) {
              shellErrno = 0;
              shellErrno = builtinCmds[i].cmd(cmdArgc, cmdArgv);
            }
          }
        }
#endif

        ps = ps1;
      } else {
        ps = ps2;
      }
      freeParseResult(result);
    }
  }
 
  return 0;
}
