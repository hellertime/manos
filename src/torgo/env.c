/**
 * env.c
 * Shell environment
 */

#include <manos.h>

#include <torgo/env.h>
#include <torgo/dstring.h>

/*
 * EnvVar = (String, String, EnvVar)
 *
 * Stores the name and value of a shell environment variable.
 * Set with the shell command "set 'name' = 'value';"
 */
typedef struct EnvVar {
  String*        name;
  String*        value;
  struct EnvVar* next;
} EnvVar;

/*
 * mkEnvVar :: String -> String -> EnvVar
 *
 * Allocate a new EnvVar, and copy the 'name' and 'value' passed into it.
 */
EnvVar* mkEnvVar(const String *name, const String *value) {
  EnvVar *var = kmalloc(sizeof *var);
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
void freeEnvVar(EnvVar *var) {
  freeString(var->name);
  freeString(var->value);
  kfree(var);
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
Env* mkEnv(void) {
  Env *env = kmalloc(sizeof *env);
  env->vars = NULL;
  return env;
}

/*
 * clearEnv :: Env -> ()
 *
 * Clear the internal memory of the Env
 */
void clearEnv(Env *env) {
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
void freeEnv(Env *env) {
  clearEnv(env);
  kfree(env);
}

/*
 * addEnvVar :: Env -> String -> String -> String
 *
 * Adds the 'name', 'value' pair to the environment. 
 * Returns the 'name' (new address) on success. NULL on failure.
 */
const String* addVarEnv(Env *env, const String *name, const String *value) {
  EnvVar *var = mkEnvVar(name, value);
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
const String* lookupVarEnv(Env *env, const String *name) {
  EnvVar *vars = env->vars;
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
const String* updateVarEnv(Env *env, const String *name, const String *value) {
  EnvVar *stub = mkEnvVar(name, value);
  if (! stub) return NULL;

  EnvVar *vars = env->vars;
  while (vars) {
    if (matchString(vars->name, name)) {
      String *old = vars->value;
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
void unsetVarEnv(Env *env, const String *name) {
  EnvVar *dummy = env->vars;
  env->vars = dummy;

  while (dummy->next) {
    if (matchString(dummy->next->name, name)) {
      EnvVar *delete = dummy->next;
      dummy->next = dummy->next->next;
      freeEnvVar(delete);
    } else {
      dummy->next = dummy->next->next;
    }
  }

  env->vars = dummy->next;
}
