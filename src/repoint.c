#include "global.h"
#include "string.h"
#include "modding.h"
#include "recomputils.h"
#include "rt64_extended_gbi.h"
#include "stdbool.h"
#include "z64animation.h"
#include "helpers.h"
#include "objectmanager.h"

#define SEGMENT_GAMEPLAY_KEEP 4
#define SEGMENT_FIELD_OR_DANGEON_KEEP 5

ObjectId gFieldOrDangeonKeep;

void Repoint_setFieldOrDangeonKeep(ObjectId id) {
    gFieldOrDangeonKeep = id;
}

void Repoint_unsetFieldOrDangeonKeep() {
    gFieldOrDangeonKeep = 0;
}

RECOMP_EXPORT void GlobalObjects_rebaseDL(void *newBase, Gfx *globalPtr, unsigned targetSegment) {
    if (isSegmentedPtr(newBase)) {
        recomp_printf("GlobalObjects_rebaseDL: Incorrectly passed in non-global pointer 0x%X as newBase\n", newBase);
        return;
    }

    if (isSegmentedPtr(globalPtr)) {
        recomp_printf("GlobalObjects_rebaseDL: Incorrectly passed in non-global pointer 0x%X as globalPtr\n", globalPtr);
        return;
    }

    unsigned opcode;

    unsigned currentSegment;

    bool isEndDl = false;

    while (!isEndDl) {
        opcode = globalPtr->words.w0 >> 24;

        currentSegment = SEGMENT_NUMBER(globalPtr->words.w1);

        switch (opcode) {
            case G_ENDDL:
                isEndDl = true;
                break;

            case G_DL:
            case G_BRANCH_Z:
                if (currentSegment == targetSegment) {
                    GlobalObjects_rebaseDL(newBase, TO_GLOBAL_PTR(newBase, globalPtr->words.w1), targetSegment);
                }

                if ((globalPtr->words.w0 >> 16 & 0xFF) == G_DL_NOPUSH) {
                    isEndDl = true;
                }
                // FALL THROUGH
            case G_VTX:
            case G_MTX:
            case G_SETTIMG:
            case G_SETZIMG:
            case G_SETCIMG:
            case G_MOVEMEM:
                if (currentSegment == targetSegment) {
                    //recomp_printf("Repointing 00x%X -> 0x%X\n", globalPtr->words.w1, TO_GLOBAL_PTR(newBase, globalPtr->words.w1));
                    globalPtr->words.w1 = (uintptr_t)TO_GLOBAL_PTR(newBase, globalPtr->words.w1);
                } else if (currentSegment == SEGMENT_GAMEPLAY_KEEP) {
                    globalPtr->words.w1 = (uintptr_t)(GlobalObjects_getGlobalGfxPtr(GAMEPLAY_KEEP, (Gfx *)(globalPtr->words.w1)));
                } else if (currentSegment == SEGMENT_FIELD_OR_DANGEON_KEEP && gFieldOrDangeonKeep) {
                    globalPtr->words.w1 = (uintptr_t)(GlobalObjects_getGlobalGfxPtr(gFieldOrDangeonKeep, (Gfx *)(globalPtr->words.w1)));
                }
                break;

            default:
                break;
        }

        globalPtr = (Gfx *)((uintptr_t)globalPtr + sizeof(Gfx));
    }
}

RECOMP_EXPORT void GlobalObjects_globalizeSegmentedDL(void *obj, Gfx *segmentedPtr) {
    if (!isSegmentedPtr(segmentedPtr) ) {
        recomp_printf("GlobalObjects_globalizeSegmentedDL: Incorrectly passed in global pointer 0x%X as segmentedPtr\n", segmentedPtr);
        return;
    }

    GlobalObjects_rebaseDL(obj, TO_GLOBAL_PTR(obj, segmentedPtr), SEGMENT_NUMBER(segmentedPtr));
}

RECOMP_EXPORT void GlobalObjects_globalizeLodLimbSkeleton(void *obj, FlexSkeletonHeader *skel) {
    if (isSegmentedPtr(skel)) {
        skel = TO_GLOBAL_PTR(obj, skel);
    }

    if (!isSegmentedPtr(skel->sh.segment)) {
        recomp_printf("GlobalObjects_globalizeLodLimbSkeleton: FlexSkeletonHeader is already global!");
        return;
    }

    skel->sh.segment = TO_GLOBAL_PTR(obj, skel->sh.segment);

    LodLimb **limbs = (LodLimb **)skel->sh.segment;

    LodLimb *limb;

    u8 limbCount = skel->sh.limbCount;

    for (u8 i = 0; i < limbCount; ++i) {
        limb = TO_GLOBAL_PTR(obj, limbs[i]);
        if (limb->dLists[0]) { // do not repoint limbs without display lists
            limb->dLists[0] = TO_GLOBAL_PTR(obj, limb->dLists[0]);
            limb->dLists[1] = TO_GLOBAL_PTR(obj, limb->dLists[1]);
        }
        limbs[i] = limb;
    }
}

RECOMP_EXPORT void GlobalObjects_globalizeStandardLimbSkeleton(void *obj, FlexSkeletonHeader *skel) {
    if (isSegmentedPtr(skel)) {
        skel = TO_GLOBAL_PTR(obj, skel);
    }

    if (!isSegmentedPtr(skel->sh.segment)) {
        recomp_printf("GlobalObjects_globalizeStandardLimbSkeleton: FlexSkeletonHeader is already global!");
        return;
    }

    skel->sh.segment = TO_GLOBAL_PTR(obj, skel->sh.segment);

    StandardLimb **limbs = (StandardLimb **)skel->sh.segment;

    StandardLimb *limb;

    u8 limbCount = skel->sh.limbCount;

    for (u8 i = 0; i < limbCount; ++i) {
        limb = TO_GLOBAL_PTR(obj, limbs[i]);
        if (limb->dList) { // do not repoint limbs without display lists
            limb->dList = TO_GLOBAL_PTR(obj, limb->dList);
        }
        limbs[i] = limb;
    }
}
