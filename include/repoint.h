#ifndef __REPOINT_H__
#define __REPOINT_H__
#include "global.h"

void Repoint_setFieldOrDangeonKeep(ObjectId id);

void Repoint_unsetFieldOrDangeonKeep();

void GlobalObjects_rebaseDL(void *newBase, Gfx *globalPtr, unsigned targetSegment);

void GlobalObjects_globalizeSegmentedDL(void *obj, Gfx *segmentedPtr);

void GlobalObjects_globalizeLodLimbSkeleton(void *obj, FlexSkeletonHeader *skel);

void GlobalObjects_globalizeStandardLimbSkeleton(void *obj, FlexSkeletonHeader *skel);

#endif
