/**
 * env.c
 * Shell environment
 */

#include <shell/env.h>
#include <shell/mem.h>
#include <shell/string.h>

/*
 * EnvVar = (String, String, EnvVar)
 *
 * Stores the name and value of a shell environment variable.
 * Set with the shell command "set 'name' = 'value';"
 */
struct EnvVar {
  struct String *name;
  struct String *value;
  struct EnvVar *next;
};

/*
 * mkEnvVar :: String -> String -> EnvVar
 *
 * Allocate a new EnvVar, and copy the 'name' and 'value' passed into it.
 */
struct EnvVar* mkEnvVar(const struct String *name, const struct String *value) {
  struct EnvVar *var = malloc(sizeof *var);
  if (! var) return NULL;

  var->name = copyString(name);
  var->value = copyString(value);
  var->next = NULL;
  return var;
}

/*
 * freeEnvVar :: EnvVar -> ()
 *
 * Release memory associated with the EnvVar
 */
void freeEnvVar(struct EnvVar *var) {
  freeString(var->name);
  freeString(var->value);
  free(var);
}

/*
 * Env = [EnvVar]
 *
 * Maintains a list of EnvVar pairs.
 * Storage is a linked list with new values appended to
 * the front of the list. So that variables can be shadowed
 * easily (at the cost of wasted space).
 */
struct Env {
  struct EnvVar *vars;
};

/*
 * mkEnv :: Env
 *
 * Allocate a new Env.
 */
struct Env* mkEnv(void) {
  struct Env *env = malloc(sizeof *env);
  env->vars = NULL;
  return env;
}

/*
 * clearEnv :: Env -> ()
 *
 * Clear the internal memory of the Env
 */
void clearEnv(struct Env *env) {
  while (env->vars) {
    struct EnvVar *v = env->vars->next;
    freeEnvVar(env->vars);
    env->vars = v;
  }
}

/*
 * freeEnv :: Env -> ()
 *
 * Release the memory allocated to the Env
 */
void freeEnv(struct Env *env) {
  clearEnv(env);
  free(env);
}

/*
 * addEnvVar :: Env -> String -> String -> String
 *
 * Adds the 'name', 'value' pair to the environment. 
 * Returns the 'name' (new address) on success. NULL on failure.
 */
const struct String* addVarEnv(struct Env *env, const struct String *name, const struct String *value) {
  struct EnvVar *var = mkEnvVar(name, value);
  if (! var) return NULL;

  var->next = env->vars;
  env->vars = var;
  return var->name;
}

/*
 * lookupVarEnv :: Env -> String -> String
 *
 * Lookup 'name' in 'env' by searching down the list of vars.
 * If 'name' is found, the value is returned. Otherwise NULL.
 */
const struct String* lookupVarEnv(struct Env *env, const struct String *name) {
  struct EnvVar *vars = env->vars;
  while (vars) {
    if (matchString(vars->name, name))
      return vars->value;

    vars = vars->next;
  }

  return NULL;
}

/*
 * updateEnvVar :: Env -> String -> String
 *
 * Lookup 'name' in 'env' and replace it with the new value.
 */
const struct String* updateVarEnv(struct Env *env, const struct String *name, const struct String *value) {
  struct EnvVar *stub = mkEnvVar(name, value);
  if (! stub) return NULL;

  struct EnvVar *vars = env->vars;
  while (vars) {
    if (matchString(vars->name, name)) {
      struct String *old = vars->value;
      vars->value = stub->value;
      stub->value = old;
      freeEnvVar(stub);
      return value;
    }
  }

  stub->next = env->vars;
  env->vars = stub;
  return stub->value;
}

/*
 * unsetVarEnv :: Env -> String -> ()
 *
 * Remove 'name' from vars. This removes all instances in the list.
 */
void unsetVarEnv(struct Env *env, const struct String *name) {
  struct EnvVar *dummy = env->vars;
  env->vars = dummy;

  while (dummy->next) {
    if (matchString(dummy->next->name, name)) {
      struct EnvVar *delete = dummy->next;
      dummy->next = dummy->next->next;
      freeEnvVar(delete);
    } else {
      dummy->next = dummy->next->next;
    }
  }

  env->vars = dummy->next;
}
