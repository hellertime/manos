/*
 * devramfs.c - a RAM fs
 */
#include <assert.h>
#include <string.h>

#include <libc.h>

#include <manos/dev.h>
#include <manos/err.h>
#include <manos/path.h>
#include <manos/portal.h>
#include <manos/types.h>

#define BLOCK_SIZE 4096

struct Block {
  int used;
  char b[BLOCK_SIZE];
  struct Block *next;
};

static struct Block* mkBlock() {
  struct Block *b = mallocz(sizeof *b);
  return b;
}

struct F {
  char *name;
  struct Fid fid;
  Mode mode;
  Offset length;
  struct F *child, *sibling;
  struct Block *root;
};

typedef enum {
  FidRoot,
} RamfsFidEnt;

static struct F root = {
  .name = "/",
  .fid = { FidRoot, FID_ISDIR },
  .mode = 0444,
  .length = 0,
  .child = 0,
  .sibling = 0,
  .root = 0
};

static struct F* mkF(char *name) {
  struct F *f = mallocz(sizeof *f);
  
  f->name = strdup(name);
  f->child = 0;
  f->sibling = 0;
  f->root = 0;
  return f;
}

#if 0
static void freeF(struct F *f) {
  free(f->name);
  free(f);
}
#endif

struct Portal* fromF(struct F* f) {
  struct Portal *p = mkPortal(DEV_DEVRAMFS);
  p->name = strdup(f->name);
  p->fid = f->fid;
  return p;
}

static struct F* fromPortal(struct Portal *p) {
  assert(p->devId == DEV_DEVRAMFS && "Unexpected type of portal");
  return (struct F*)p->fid.tag;
}

static struct Portal* attachRamfs(char *path) {
  struct Portal *p = attachDev(DEV_DEVRAMFS, path);
  p->fid.type = FID_ISDIR;
  p->fid.tag = (uint32_t)&root;
  return p;
}

/*
 * In the RamFS. All paths must be absolute. Starting from 'root'
 */
static struct Portal *walk (struct Portal *p, char *component, struct DevInfo *info) {
  struct F *f = fromPortal(p);
  int walkSiblings = f->fid.type & FID_ISFILE;
  
  while (f) {
	if (streq(f->name, component))
	  break;
	
    if (walkSiblings) {
      if (!f->sibling)
        return NULL;
    
      f = f->sibling;
    } else {
      if (!f->child)
        return NULL;
    
      f = f->child;
    }
  }

  struct Portal *p2 = mkPortal(DEV_DEVRAMFS);  
  p2->name = strdup(component);
  p2->mode = f->mode;
  p2->fid.type = f->fid.type;
  p2->fid.tag = (uint32_t)f;
  
  info->devId = DEV_DEVRAMFS;
  info->fid = p2->fid;
  info->mode = f->mode;
  info->length = f->length;
  
  return p2;
}

static struct Portal* openRamfs(struct Portal *p, OMode mode) {
  p = openDev(p, mode);
  
  struct Path *path = mkPath(p->name);
  struct Portal *t = p;
  struct DevInfo info;

  for (int i = 0; i < path->nhistory; i++) {  
    t = walk(p, path->history[i]->name, &info);
    if (!t) {
      freePortal(p);
      return NULL;
    }
    freePortal(p);
    p = t;
  }
  
  return p;
}

static void closeRamfs(struct Portal *p) {
  UNUSED(p);
}

static struct Block* seekBlockRamfs(struct F *f, Offset offset, Offset *boffset) {
  struct Block *b = f->root;
  Offset rem = offset;
  
  while (b && rem > BLOCK_SIZE) {
    b = b->next;
    rem -= BLOCK_SIZE;
  }
  
  *boffset = rem;
  return b;
}

static struct Block* growFileRamfs(struct F *f, Offset offset, uint32_t size, Offset *boffset) {
  struct Block *b = f->root;
  Offset rem = offset;
  
  if (!size)
    return NULL;
  
  if (!b)
    f->root = mkBlock(); /* first block */
  
  while (rem > BLOCK_SIZE) {
    if (!b->next)
      b->next = mkBlock();
    
    rem -= BLOCK_SIZE;
    b = b->next;
  }
  
  struct Block *startBlock = b;
  *boffset = rem;
  
  /* calculate size of fill area -- add rem to account for current block offset*/
  rem = rem + size;
  while (rem > BLOCK_SIZE) {
    if (!b->next)
      b->next = mkBlock();
    
    rem -= BLOCK_SIZE;
    b = b->next;
  }
  
  /* spill into one more block, create if needed */
  if (rem && !b->next)
    b->next = mkBlock();
  
  return startBlock;
}

static int32_t readFileRamfs(struct F* f, void *buf, uint32_t size, Offset offset, Err *err) {
  if (offset + size > f->length) {
    *err = E_SIZE;
    return -1;
  }
		
  Offset blockOffset = 0;
  struct Block *b = seekBlockRamfs(f, offset, &blockOffset);
  if (!b) {
    *err = E_SIZE;
	return -1;
  }
	    
  uint32_t bytes = size;
  char *cin = b->b, *cout = buf;
  while (bytes) {
    while (blockOffset < BLOCK_SIZE) {
      *cout++ = *cin++;
	  blockOffset++;
	  bytes--;
    }
    
    if (bytes) {
      b = b->next;
      cin = b->b;
      blockOffset = 0;
    }
  }
	    
  *err = E_OK;
  return size;
}

static int32_t readDirRamfs(struct F *f, void *buf, uint32_t size, Offset offset, Err *err) {
  char *b = buf;
  uint32_t xsize;
  uint32_t bytes = size;
    
  while (bytes && f->child) {
    int n = strlen(f->name) + 1; /* \n */
    if (n < bytes) n = bytes;
    f = f->child;
    memcpy(b, f->name, n - 1);
    b += (n - 1);
    *b++ = '\n';
    xsize += n;
    bytes -= n;
  }
  *err = E_OK;
  return xsize;
}

static int32_t readRamfs(struct Portal *p, void *buf, uint32_t size, Offset offset, Err *err) {
  struct F* f = fromPortal(p);
  
  if (f->fid.type & FID_ISFILE) {
    return readFileRamfs(f, buf, size, offset, err);
  } else {
    return readDirRamfs(f, buf, size, offset, err);
  }
}

static int32_t writeFileRamfs(struct F *f, void *buf, uint32_t size, Offset offset, Err *err) {
  if (f->fid.type & FID_ISDIR) {
    *err = E_PERM;
    return -1;
  }
  
  Offset blockOffset = 0;
  struct Block *b = growFileRamfs(f, offset, size, &blockOffset);
  
  uint32_t bytes = size;
  char *cin = buf, *cout = b->b;
  while (bytes) {
    while (blockOffset < BLOCK_SIZE) {
      *cout++ = *cin++;
  	  blockOffset++;
      bytes--;
    }
      
    if (bytes) {
      b = b->next;
      cout = b->b;
      blockOffset = 0;
    }
  }
  	    
  *err = E_OK;
  return size;
}

static int32_t writeRamfs(struct Portal *p, void *buf, uint32_t size, Offset offset, Err *err) {
  struct F *f = fromPortal(p);
  return writeFileRamfs(f, buf, size, offset, err);
}


/*
 * More magic.
 * 
 * Haven't figured out the dir/file protocol yet. so to create a directory pass
 * in a path of /.../file/. <-- trailing dot
 * this inband signalling will be used at the protocol layer. the upper shell
 * will have specialized commands for directory creation.
 */
static Err createRamfs(struct Portal *p, char *name, OMode mode, Perm perm) {
  struct Path *path = mkPath(name);
  
  int targetIdx = path->nhistory - 1;
  char *target = path->history[targetIdx]->name;
  int mkdirs = 0;
  if (*target == '.') {
    mkdirs = 1;
    target = path->history[--targetIdx]->name;
  }
  
  struct Portal *t = p;
  struct DevInfo info;
  for (int i = 0; i < targetIdx; i++) {  
    struct Portal *t2 = walk(t, path->history[i]->name, &info);
    
    if (!t2) {
      struct F *parent = fromPortal(t);
      if (parent->fid.type & FID_ISDIR && mkdirs) {
        struct F *sibling = mkF(path->history[i]->name);
        sibling->fid.type = FID_ISDIR;
        sibling->fid.tag = (uint32_t)sibling;
        sibling->length = 0;
        sibling->root = 0;
        sibling->child = 0;
        sibling->sibling = 0;
        while (parent->sibling)
          parent = parent->sibling;
        
        parent->sibling = sibling;
        t = fromF(sibling);
        continue;
      } else {
        freePath(path);
        return E_NOTFOUND;
      }
    }
    
    t = t2;
  }
  
  freePath(path);
  
  struct F *parent = fromPortal(t);
  struct F *f = mkF(target);
  f->fid.tag = (uint32_t)f;
  f->length = 0;
  f->child = 0;
  f->sibling = 0;
  
  if (mkdirs) {
    f->fid.type = FID_ISDIR;
    f->root = 0;
  } else {
    f->fid.type = FID_ISFILE;
  }
  
  while (parent->child)
    parent = parent->child;
  
  parent->child = f;
  p = fromF(f);
  return E_OK;
}

static Err getInfoRamfs(struct Portal *p, struct DevInfo *info) {
  struct F *f = fromPortal(p);
  info->devId = DEV_DEVRAMFS;
  info->name = p->name;
  info->fid = p->fid;
  
  if (p->fid.type & FID_ISDIR) {
    struct F *fc = f;
    uint32_t l = 0;
    while (fc->child) {
      l += strlen(fc->child->name) + 1; /* plus \n */
      fc = fc->child;
    }
    info->length = l;
  } else {
    info->length = f->length;
  }
  info->mode = f->mode;
  return E_OK;
}

struct Dev ramfsDev = {
  .id = DEV_DEVRAMFS,
  .name = "ramfs",
  .init = initDev,
  .reset = resetDev,
  .shutdown = shutdownDev,
  .attach = attachRamfs,
  .create = createRamfs,
  .open = openRamfs,
  .close = closeRamfs,
  .getInfo = getInfoRamfs,
  .setInfo = setInfoDev,
  .read = readRamfs,
  .write = writeRamfs
};
