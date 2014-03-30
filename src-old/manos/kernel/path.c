/*
 * path.c - represent a path as an object.
 */
#include <assert.h>

#include <libc.h>

#include <manos/path.h>
#include <manos/portal.h>

/*
 * mkPath :: String -> Path
 * 
 * A path contains the trail of Portals it crosses.
 * For now path must always be absolute and canonical.
 */
struct Path* mkPath(char *path) {
  char *c, *c0;
  struct Path *p = malloc(sizeof *p);
  
  p->p = strdup(path);
  
  /*
   * Count the '/' in the path to allocate the Portal array
   */
  c = p->p;
  unsigned components = (*c == '/') ? 0 : 1;
  while (*c) {
    if(*c == '/')
	  components++;
    c++;
    while (*c && *c != '/') {
      c++;
    }
  }
  
  p->history = malloc(sizeof *p->history);
  p->nhistory = components;
  
  c0 = c = path + (*path == '/'); /* skip '/' */
  components = 0;
  while (*c0) {
    while (*c && *c != '/')
      c++;
    
    *c = 0;
    p->history[components] = mkPortal(0); /* skip devId for now */
    p->history[components]->name = strdup(c0);
    
    if (components == p->nhistory)
      break;
    
    *c++ = '/';
    c0 = c;
    components++;
  }
  
  return p;
}

void freePath(struct Path *path) {
  for (unsigned i = 0; i < path->nhistory; i++) {
    freePortal(path->history[i]);
  }
  free(path->history);
  free(path);
}
