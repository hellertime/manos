#ifndef SHELL_ENV_H
#define SHELL_ENV_H

/**
 * env.h
 * Shell environment
 */

struct Env;

struct Env* mkEnv(void);
void freeEnv(struct Env *env);

const struct String* addVarEnv(struct Env *env, const struct String *name, const struct String *value);
const struct String* updateVarEnv(struct Env *env, const struct String *name, const struct String *value);
const struct String* lookupVarEnv(struct Env *env, const struct String *name);
void unsetVarEnv(struct Env *env, const struct String *name);

#endif /* ! SHELL_ENV_H */
