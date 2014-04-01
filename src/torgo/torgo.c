/**
 * shell.c
 *
 * Provides the command evaluation and interactive bits of the shell.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <manos.h>

#include <torgo/charbuf.h>
#include <torgo/commands.h>
#include <torgo/env.h>
#include <torgo/parser.h>

#define MAX_FD 256

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

  fputs(promptStr, stdout);

  if (promptStr[strlen(promptStr) - 1] != ' ')
    fputc(' ', stdout);
  
  fflush(stdout);

  while (keepReading && (readMax == -1 || readMax > 0)) {
    int c = fgetc(stdin);
    switch (c) {
      case EOF:
        keepReading = 0;
        break;
      case '\n':
        keepReading = 0;
        /* fall through */
      default:
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
void populateCmdArgsShell(Env *env, ParseResult *result, int *argc, char ***argv) {
  int argc_ = getLengthParseResult(result);
  char **argv_ = kmallocz(argc_ * sizeof *argv_);

  ParseTokenIterator *tokens = getParseTokenIteratorParseResult(result);
  CharBuf *tokenBuilder = mkCharBuf(32);
  CharBuf *varBuilder = mkCharBuf(32);

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
    argv_[i] = kmallocz(strlen(arg) + 1);
    memcpy(argv_[i], arg, strlen(arg));
  }

  freeParseTokenIterator(tokens);
  *argc = argc_;
  *argv = argv_;
}

#include "fakeproc.h"

/*
 * main
 *
 * Main driver loop. Loads input from the prompt, and
 * drives the parse. Certain builtins that need to effect
 * the environment are defined here.
 */
int torgo_main(int argc, char *argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  const char *ps1 = "torgo > ", *ps2 = "> ";
  Shell *shell = mkShell();
  int shellErrno = 0;
  
  setvbuf(stdin, NULL, _IONBF, 0);

#ifdef HAS_FAKEPROC
  initDeviceTable();
  slash = deviceTable[fromDeviceId(DEV_DEVROOT)]->attach("");
  dot   = deviceTable[fromDeviceId(DEV_DEVROOT)]->attach("");
#endif
  
  const char *ps = ps1;
  while (shell->state == ShellStateRun) {
    const CharBuf *input = readPromptShell(shell, ps, 32);

    if (isEmptyCharBuf(input)) {
      shell->state = ShellStateEOF;
      if (hasUnparsedInputParser(shell->parser)) {
        fputs("error: unexpected end-of-file\n", stdout);
      }
      fputs("\n", stdout);
      break;
    }

    addInputParser(shell->parser, fromCharBuf(input));

    while (hasUnparsedInputParser(shell->parser)) {
      ParseResult *result = parseInputParser(shell->parser);

      if (isCompleteParseResult(result)) {
        int cmdArgc;
        char **cmdArgv;
        populateCmdArgsShell(shell->env, result, &cmdArgc, &cmdArgv);

        shellErrno = 0;
        shellErrno = fakeExecv(cmdArgv[0], cmdArgc, cmdArgv);

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
      fflush(stdout);

      freeParseResult(result);
    }
  }
 
  return 0;
}
