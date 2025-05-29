#ifndef __OBJECT_MANAGER_H__
#define __OBJECT_MANAGER_H__

#include "global.h"

// A workaround for object dependency issues
// This manager keeps objects loaded and repoints as needed

// Returns a pointer to the object with the given id
// if it was not already loaded, it is loaded here
void *ZGlobalObj_getGlobalObject(ObjectId id);

Gfx *ZGlobalObj_getGlobalGfxPtr(ObjectId id, Gfx *segmentedPtr);

int ZGlobalObj_getObjIdFromVrom(uintptr_t vromStart, unsigned long *out);

void *ZGlobalObj_getGlobalObjectFromVrom(uintptr_t vromStart);

#endif
