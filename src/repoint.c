#include "global.h"
#include "string.h"
#include "modding.h"
#include "recomputils.h"
#include "rt64_extended_gbi.h"
#include "stdbool.h"
#include "z64animation.h"
#include "helpers.h"





RECOMP_EXPORT void ZGlobalObj_globalizeDL(void *obj, Gfx *segmentedPtr) {
    if (!isSegmentedPtr(segmentedPtr)) {
        return;
    }

    u32 segmentOffset = SEGMENT_OFFSET(segmentedPtr);

    u32 segment = SEGMENT_NUMBER(segmentedPtr);

    Gfx *globalPtr = TO_GLOBAL_PTR(obj, segmentedPtr);

    u8 opcode;

    bool isEndDl = false;

    while (!isEndDl) {
        opcode = globalPtr->words.w0 >> 24;

        switch (opcode) {
            case G_ENDDL:
                isEndDl = true;
                break;

            case G_DL:
                if (SEGMENT_NUMBER(globalPtr->words.w1) == segment) {
                    ZGlobalObj_globalizeDL(obj, (Gfx *)(globalPtr->words.w1));
                }

                if ((globalPtr->words.w0 >> 16 & 0xFF) == G_DL_NOPUSH) {
                    isEndDl = true;
                }
                // FALL THROUGH
            case G_VTX:
            case G_MTX:
            case G_SETTIMG:
            case G_MOVEMEM:
                if (SEGMENT_NUMBER(globalPtr->words.w1) == segment) {
                    ZGlobalObj_globalizeGfx(obj, (Gfx *)globalPtr->words.w1);
                }
                break;

            default:
                break;
        }

        globalPtr = (Gfx *)((uintptr_t)globalPtr + sizeof(Gfx));
    }
}

RECOMP_EXPORT void ZGlobalObj_globalizeLodLimbSkeleton(void *obj, FlexSkeletonHeader *skel) {
    if (!isSegmentedPtr(skel)) {
        return;
    }

    FlexSkeletonHeader *skelGlobal = TO_GLOBAL_PTR(obj, skel);

    LodLimb **limbs = TO_GLOBAL_PTR(obj, skelGlobal->sh.segment);

    LodLimb *limb;

    u8 limbCount = skelGlobal->dListCount;

    for (u8 i = 0; i < limbCount; ++i) {
        limb = TO_GLOBAL_PTR(obj, limbs[i]);
        if (limb->dLists[0]) { // do not repoint limbs without display lists
            limb->dLists[0] = TO_GLOBAL_PTR(obj, limb->dLists[0]);
            limb->dLists[1] = TO_GLOBAL_PTR(obj, limb->dLists[1]);
        }
        limbs[i] = limb;
    }
}

RECOMP_EXPORT void ZGlobalObj_globalizeStandardLimbSkeleton(void *obj, FlexSkeletonHeader *skel) {
    if (!isSegmentedPtr(skel)) {
        return;
    }

    FlexSkeletonHeader *skelGlobal = TO_GLOBAL_PTR(obj, skel);

    StandardLimb **limbs = TO_GLOBAL_PTR(obj, skelGlobal->sh.segment);

    StandardLimb *limb;

    u8 limbCount = skelGlobal->dListCount;

    for (u8 i = 0; i < limbCount; ++i) {
        limb = TO_GLOBAL_PTR(obj, limbs[i]);
        if (limb->dList) { // do not repoint limbs without display lists
            limb->dList = TO_GLOBAL_PTR(obj, limb->dList);
        }
        limbs[i] = limb;
    }
}
