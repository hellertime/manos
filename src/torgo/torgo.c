/**
 * shell.c
 *
 * Provides the command evaluation and interactive bits of the shell.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libc.h>

#include <manos/dev.h>
#include <manos/err.h>
#include <manos/path.h>
#include <manos/portal.h>
#include <manos/types.h>

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

  shell->readBuf = mkCharBuf(32);
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
void populateCmdArgsShell(struct Env *env, struct ParseResult *result, int *argc, char ***argv) {
  int argc_ = getLengthParseResult(result);
  char **argv_ = malloc(argc_ * sizeof *argv_);

  struct ParseTokenIterator *tokens = getParseTokenIteratorParseResult(result);
  struct CharBuf *tokenBuilder = mkCharBuf(32);
  struct CharBuf *varBuilder = mkCharBuf(32);

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
 * Setup IO subsystem. Real hackish. A vector maps 'file descriptors' to Portals.
 * We don't have processes, so there is little thought on how procs will interact
 * yet. We'll get there. Hopefully this semester.
 */

/*
 * Each device gets an entry in this device table.
 * When you open a file in the namespace, the Portals
 * in this table are cloned into the descriptorTable.
 */

struct MountTable {
  char *path;
  DevId id;
} mountTable[] = {
  { "/dev/led",  DEV_DEVLED   },
  { "/dev/swpb", DEV_DEVSWPB  },
  { "/dev/ramfs",DEV_DEVRAMFS }
};

struct Dev* deviceTable[] = {
  &ledDev,
  &swpbDev,
  &ramfsDev,
};

struct Portal* descriptorTable[MAX_FD] = {0};

static int fromDevId(DevId id) {
  for (unsigned i = 0; i < COUNT_OF(deviceTable); i++) {
    if (deviceTable[i]->id == id)
      return (int)i;
  }
  return -1;
}

static struct Portal* fromPath(char *path) {
  struct Portal *p = NULL;
  
  for (unsigned i = 0; i < COUNT_OF(mountTable); i++) {
    char *mountPoint = mountTable[i].path;
    if (nstreq(mountPoint, path, strlen(mountPoint))) {
      DevId id = mountTable[i].id;
      int idx = fromDevId(id);
      if (idx >= 0) {
        char *devPath = path + strlen(mountPoint);
        if (*devPath == '/') devPath++;
        if (!devPath) devPath = "";
        p = deviceTable[idx]->attach(devPath);
      }
    }
  }
  
  return p;
}

static int shellOpen(char *path) {
  /* OK. Magic IO commands for now. Boy would I love a completed IO layer and a real shell to connect with it */
  if (*path != '/') {
	printf("usage: open ABS_PATH");
    return E_BADARG;
  }
	
  struct Portal *p = fromPath(path);

  if (!p) {
    return E_NOMOUNT;
  }
  
  unsigned i; /* i will be our file descriptor */
  for (i = 0; i < MAX_FD; i++) {
    if (descriptorTable[i] == NULL) {
      descriptorTable[i] = p;
	  break;
	}
  }
	            
  if (i == MAX_FD) {
	freePortal(p);
	return E_NOFD;
  }
    
  return i;
}

int shellMkdir(char *path) {
	if (*path != '/') {
      printf("usage: mkdir ABS_PATH");
      return E_BADARG;
	}
	
	struct Portal *p = fromPath(path);
	
	if (!p)
	  return E_NOMOUNT;
	
	path = p->name;
	
	char *dirPath = malloc(strlen(path) + 2);
	memcpy(dirPath, path, strlen(path));
	char *c = dirPath + strlen(path);
	
	if (*c != '/') {
	  *c++ = '/';
	}
	
	*c++ = '.';
	*c = 0;
	
	struct Dev *dev = deviceTable[fromDevId(p->devId)];
	int res = dev->create(p, dirPath, 0, 0);
	freePortal(p);
	return res;
}

int shellTouch(char *path) {
  if (*path != '/') {
    printf("usage: touch ABS_PATH");
    return E_BADARG;
  }
  
  struct Portal *p = fromPath(path);
  
  if (!p)
    return E_NOMOUNT;
  
  path = p->name;
  
  struct Dev *dev = deviceTable[fromDevId(p->devId)];
  int res = dev->create(p, path, 0, 0);
  freePortal(p);
  return res;
}

int shellStat(int fd) {
  if (fd < 0 || fd >= MAX_FD) {
    return E_BADARG;
  }
  
  struct Portal *p = descriptorTable[fd];
  
  if (!p) {
    return E_PERM;
  }
  
  struct Dev *dev = deviceTable[fromDevId(p->devId)];
  p = dev->open(p, OMODE_READ);
  
  struct DevInfo info;
  Err err = dev->getInfo(p, &info);
  
  /* remember info->name is currently alaised to p->name
   * so we don't need to release the RAM, but we also cannot
   * discard p at this point.
   */
  
  if (err == E_OK) {
	printf("Name: %s\n", info.name);
    printf("DevId: %c\n", info.devId);
    printf("Fid: (type: %d, tag: %ld)\n", info.fid.type, info.fid.tag);
    if (info.fid.type & FID_ISDIR)
      printf("  Flag: FID_ISDIR\n");
    if (info.fid.type & FID_ISFILE)
      printf("  Flag: FID_ISFILE\n");
    printf("Length: %ld\n",info.length);
    printf("Mode: %o\n", info.mode);
    printf("ATime: %ld\n", info.accessTime);
    printf("Mtime: %ld\n", info.modTime);
    printf("Owner: %s\n", info.owner);
    printf("Group: %s\n", info.group);
  }
  
  return err;
}

/*
 * Demo of a device read. We don't have files or pipelines so output
 * just goes to stdout. So no need to pass a buffer or anything
 * more exotic.
 */
int shellRead(int fd, uint32_t size, uint32_t offset) {
  if (fd < 0 || fd >= MAX_FD) {
    return E_BADARG;
  }
  struct Portal *p = descriptorTable[fd];
  
  struct Dev *dev = deviceTable[fromDevId(p->devId)];
  p = dev->open(p, OMODE_READ);
  
  if (!p) {
    return E_PERM;
  }
  
  char *buf = malloc(size + 1);
  
  Err err;
  int32_t bytes = dev->read(p, buf, size, offset, &err);
  dev->close(p);
  if (bytes == -1) {
    free(buf);
    return err;
  }
  
  buf[bytes] = 0;
  printf("%s", buf);
  free(buf);
  return E_OK;
}

int shellWrite(int fd, char *buf, uint32_t size, uint32_t offset) {
  if (fd < 0 || fd > MAX_FD) {
    return E_BADARG;
  }
  
  struct Portal *p = descriptorTable[fd];
  struct Dev *dev = deviceTable[fromDevId(p->devId)];
  p = dev->open(p, OMODE_WRITE);
  
  if (!p) {
    return E_PERM;
  }
  
  Err err;
  int32_t bytes = dev->write(p, buf, size, offset, &err);
  dev->close(p);
  
  if (bytes == -1) {
    return err;
  }
  
  return E_OK;
}

/*
 * main
 *
 * Main driver loop. Loads input from the prompt, and
 * drives the parse. Certain builtins that need to effect
 * the environment are defined here.
 */
int main(int argc, char *argv[]) {
  const char *ps1 = "torgo > ", *ps2 = "> ";
  struct Shell *shell = mkShell();
  int shellErrno = 0;
  
  setvbuf(stdin, NULL, _IONBF, 0);
  
  for (unsigned int i = 0; i < COUNT_OF(deviceTable); i++) {
    deviceTable[i]->init();
  }
  
  const char *ps = ps1;
  while (shell->state == ShellStateRun) {
    const struct CharBuf *input = readPromptShell(shell, ps, 32);

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
      struct ParseResult *result = parseInputParser(shell->parser);

      if (isCompleteParseResult(result)) {
        int cmdArgc;
        char **cmdArgv;
        populateCmdArgsShell(shell->env, result, &cmdArgc, &cmdArgv);

        /*
         * Some commands need shell environment access and so cannot be passed of to 'exec' at the moment
         */
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
          printf("Last status (%d): %s\n", shellErrno, fromErr(shellErrno));
        } else if (cmdArgc == 2 && streq(cmdArgv[0], "mkdir")) {
          shellErrno = shellMkdir(cmdArgv[1]);
        } else if (cmdArgc == 2 && streq(cmdArgv[0], "touch")) {
          shellErrno = shellTouch(cmdArgv[1]);
        } else if (cmdArgc == 2 && streq(cmdArgv[0], "open")) {
          int fd = shellOpen(cmdArgv[1]);
          if (fd >= 0) {
            printf("%d\n", fd);
            shellErrno = E_OK;
          } else {
            shellErrno = fd;
          }
        } else if (cmdArgc == 2 && streq(cmdArgv[0], "stat")) {
          int fd = atoi(cmdArgv[1]);
          shellErrno = shellStat(fd);
        } else if (cmdArgc >= 3 && streq(cmdArgv[0], "read")) {
          int fd = atoi(cmdArgv[1]);
          uint32_t size = atol(cmdArgv[2]);
          uint32_t offset = 0;
          
          if (cmdArgc == 4) {
            offset = atol(cmdArgv[3]);
          }
          
          shellErrno = shellRead(fd, size, offset);
        } else if (cmdArgc == 3 && streq(cmdArgv[0], "write")) {
          int fd = atoi(cmdArgv[1]);
          shellErrno = shellWrite(fd, cmdArgv[2], strlen(cmdArgv[2]), 0);
        } else if (cmdArgc == 4 && streq(cmdArgv[0], "write2")) {
          int fd = atoi(cmdArgv[1]);
          uint32_t offset = atol(cmdArgv[2]);
          shellErrno = shellWrite(fd, cmdArgv[3], strlen(cmdArgv[3]), offset);
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
      fflush(stdout);

      freeParseResult(result);
    }
  }
}
