/**
 * shell.c
 *
 * Provides the command evaluation and interactive bits of the shell.
 */

#include <stdio.h>

#include <shell/charbuf.h>
#include <shell/commands.h>
#include <shell/err.h>
#include <shell/env.h>
#include <shell/mem.h>
#include <shell/parser.h>

#include <yamalloc.h>

typedef enum {
  ShellStateRun,
  ShellStateEOF,
  ShellStateExit
} ShellState;

struct Shell {
  struct Env *env;
  struct Parser *parser;
  struct CharBuf *readBuf;
  ShellState state;
};

void freeShell(struct Shell *shell);

struct Shell* mkShell(void) {
  struct Shell *shell = malloc(sizeof *shell);
  if (! shell) goto exit;

  shell->env = NULL;
  shell->parser = NULL;
  shell->readBuf = NULL;
  shell->state = ShellStateRun;

  shell->env = mkEnv();
  if (! shell->env) goto fail;

  shell->parser = mkParser();
  if (! shell->parser) goto fail;

  shell->readBuf = mkCharBuf(256);
  if (! shell->readBuf) goto fail;

exit:
  return shell;

fail:
  freeShell(shell);
  shell = NULL;
  goto exit;
}

void freeShell(struct Shell *shell) {
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
const struct CharBuf* readPromptShell(struct Shell *shell, const char *promptStr, int readMax) {
  int keepReading = 1;

  clearCharBuf(shell->readBuf);

  fputs(promptStr, stdout);

  if (promptStr[strlen(promptStr) - 1] != ' ')
    fputc(' ', stdout);

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
void populateCmdArgsShell(struct Env *env, struct ParseResult *result, int *argc, char ***argv) {
  int argc_ = getLengthParseResult(result);
  char **argv_ = malloc(argc_ * sizeof *argv_);

  struct ParseTokenIterator *tokens = getParseTokenIteratorParseResult(result);
  struct CharBuf *tokenBuilder = mkCharBuf(256);
  struct CharBuf *varBuilder = mkCharBuf(256);

  for (int i = 0; i < argc_; i++) {
    const struct String *token = getNextParseTokenIterator(tokens);
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

            struct String varStr;
            assignString(&varStr, fromCharBuf(varBuilder));
            const struct String* value = lookupVarEnv(env, &varStr);

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
    
    argv_[i] = strdup(fromCharBuf(tokenBuilder));
  }

  freeParseTokenIterator(tokens);
  *argc = argc_;
  *argv = argv_;
}

/*
 * main
 *
 * Main driver loop. Loads input from the prompt, and
 * drives the parse. Certain builtins that need to effect
 * the environment are defined here.
 */
int main(int argc, char *argv[]) {
  const char *ps1 = "e92 > ", *ps2 = "> ";
  struct Shell *shell = mkShell();
  int shellErrno = 0;

  const char *ps = ps1;
  while (shell->state == ShellStateRun) {
    const struct CharBuf *input = readPromptShell(shell, ps, 256);

    if (isEmptyCharBuf(input)) {
      shell->state = ShellStateEOF;
      if (hasUnparsedInputParser(shell->parser)) {
        fputs("error: unexpected end-of-file\n", stderr);
      }
      fputs("\n", stdout);
      break;
    }

    addInputParser(shell->parser, fromCharBuf(input));

    while (hasUnparsedInputParser(shell->parser)) {
      struct ParseResult *result = parseInputParser(shell->parser);

      if (isCompleteParseResult(result)) {
        int cmdArgc;
        char **cmdArgv;
        populateCmdArgsShell(shell->env, result, &cmdArgc, &cmdArgv);

        if (cmdArgc == 4 && streq(cmdArgv[0], "set") && streq(cmdArgv[2], "=")) {
          struct String name;
          struct String value;

          assignString(&name, cmdArgv[1]);
          assignString(&value, cmdArgv[3]);
          addVarEnv(shell->env, &name, &value);
        } else if (cmdArgc == 2 && streq(cmdArgv[0], "unset")) {
          struct String name;

          assignString(&name, cmdArgv[1]);
          unsetVarEnv(shell->env, &name);
        } else if (cmdArgc == 1 && streq(cmdArgv[0], "perror")) {
          fprintf(stdout, "Last status (%d): %s\n", shellErrno, strerror(shellErrno));
        } else {
          for (int i = 0; cmdArgc && i < numBuiltinCmds; i++) {
            if (streq(builtinCmds[i].cmdName, cmdArgv[0])) {
              shellErrno = 0;
              shellErrno = builtinCmds[i].cmd(cmdArgc, cmdArgv);
            }
          }
        }

        ps = ps1;
      } else {
        ps = ps2;
      }

      freeParseResult(result);
    }
  }
}
