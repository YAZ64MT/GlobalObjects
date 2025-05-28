#include "global.h"
#include "string.h"
#include "modding.h"
#include "recomputils.h"
#include "rt64_extended_gbi.h"
#include "stdbool.h"
#include "z64animation.h"

RECOMP_EXPORT void zglobalobj_globalize_gfx(void *obj, Gfx* segmentedPtr) {

    uintptr_t base = (uintptr_t)obj;

    u32 segmentOffset = SEGMENT_OFFSET(segmentedPtr);

    Gfx *globalPtr = (Gfx *)(base + segmentOffset);

    u8 opcode = globalPtr->words.w0 >> 24;

    switch (opcode) {
    case G_DL:
    case G_VTX:
    case G_MTX:
    case G_SETTIMG:
    case G_MOVEMEM:
        globalPtr->words.w1 = base + segmentOffset;
        break;

    default:
        break;
    }
}

RECOMP_EXPORT void zglobalobj_globalize_dl(void *obj, Gfx *segmentedPtr) {

    uintptr_t base = (uintptr_t)obj;

    u32 segmentOffset = SEGMENT_OFFSET(segmentedPtr);

    u8 segment = SEGMENT_NUMBER(segmentedPtr);

    Gfx *globalPtr = (Gfx *)(base + segmentOffset);

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
                    zglobalobj_globalize_dl(obj, (Gfx *)SEGMENT_ADDR(segment, segmentOffset));                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   ));
                }
                
                if ((globalPtr->words.w0 >> 16 & 0xFF)  == G_DL_NOPUSH) {
                    isEndDl = true;
                }
            case G_VTX:
            case G_MTX:
            case G_SETTIMG:
            case G_MOVEMEM:
                if (SEGMENT_NUMBER(globalPtr->words.w1) == segment) {
                    zglobalobj_globalize_gfx(obj, globalPtr);
                }
                break;

            default:
                break;
        }

        globalPtr = (Gfx *)((uint32_t)globalPtr + sizeof(Gfx));
    }
}

// TODO: REWRITE THESE
RECOMP_EXPORT void zglobalobj_globalizeLinkSkeleton(void *, u32 skeletonHeaderOffset, u8 targetSegment, const void *newBase) {
    u32 newBaseAddress = (u32)newBase;

    // repoint only if segmented
    if (zobj[skeletonHeaderOffset] == targetSegment) {

        u32 firstLimbOffset = SEGMENT_OFFSET(readU32(zobj, skeletonHeaderOffset));

        FlexSkeletonHeader *flexHeader = (FlexSkeletonHeader *)(&zobj[skeletonHeaderOffset]);

        writeU32(zobj, skeletonHeaderOffset, firstLimbOffset + newBaseAddress);

        LodLimb **limbs = (LodLimb **)(&zobj[firstLimbOffset]);

        //recomp_printf("Limb count: %d\n", flexHeader->sh.limbCount);
        //recomp_printf("First limb entry location: 0x%x\n", limbs);

        LodLimb *limb;
        for (u8 i = 0; i < flexHeader->sh.limbCount; i++) {
            limb = (LodLimb *)&zobj[SEGMENT_OFFSET(limbs[i])];
            if (limb->dLists[0]) { // do not repoint limbs without display lists
                limb->dLists[0] = (Gfx *)(SEGMENT_OFFSET(limb->dLists[0]) + newBaseAddress);
                limb->dLists[1] = (Gfx *)(SEGMENT_OFFSET(limb->dLists[1]) + newBaseAddress);
            }
            limbs[i] = (LodLimb *)(SEGMENT_OFFSET(limbs[i]) + newBaseAddress);
        }
    }
}

bool isBytesEqual(const void *ptr1, const void *ptr2, size_t num) {
    const u8 *a = ptr1;
    const u8 *b = ptr2;

    for (size_t i = 0; i < num; i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }

    return true;
}

RECOMP_EXPORT s32 ZobjUtils_getFlexSkeletonHeaderOffset(const u8 zobj[], u32 zobjSize) {
    // Link should always have 0x15 limbs where 0x12 have display lists
    // so, if a hierarchy exists, then this string must appear at least once
    u8 lowerHeaderBytes[] = {0x15, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00};

    const u8 LOWER_HEADER_SIZE = ARRAY_COUNT(lowerHeaderBytes);

    const u8 FLEX_HEADER_SIZE = 0xC;

    u32 index = FLEX_HEADER_SIZE - LOWER_HEADER_SIZE;

    u32 endIndex = zobjSize - FLEX_HEADER_SIZE;

    while (index < endIndex) {
        if (isBytesEqual(&zobj[index], &lowerHeaderBytes[0], LOWER_HEADER_SIZE)) {
            // account for first four bytes of header
            return index - 4;
        }

        // header must be aligned
        index += 4;
    }

    // Returning a signed value here isn't ideal
    // but in practice there should never be a zobj passed in that's >2 GB in size
    return -1;
}
