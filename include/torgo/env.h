#ifndef SHELL_ENV_H
#define SHELL_ENV_H

/**
 * env.h
 * Shell environment
 */

#include <torgo/dstring.h>

typedef struct Env Env;

Env* mkEnv(void);
void freeEnv(Env *env);

const String* addVarEnv(Env *env, const String *name, const String *value);
const String* updateVarEnv(Env *env, const String *name, const String *value);
const String* lookupVarEnv(Env *env, const String *name);
void unsetVarEnv(Env *env, const String *name);

#endif /* ! SHELL_ENV_H */
