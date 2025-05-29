#include "global.h"
#include "recompdata.h"
#include "recomputils.h"
#include "repoint.h"
#include "objectmanager.h"
#include "helpers.h"

// Maps VROM addresses to their respective object IDs
U32ValueHashmapHandle gVromToObjId;

// maps object ids to their location in memory
void *gObjIdToMemTable[OBJECT_ID_MAX] = {0};

// avoid going repointing each dl more than once
U32HashsetHandle gRepointTracker[OBJECT_ID_MAX];

void initObjectManager() {
    gVromToObjId = recomputil_create_u32_value_hashmap();

    for (size_t i = 0; i < OBJECT_ID_MAX; ++i) {
        if (gObjectTable[i].vromStart) {
            recomputil_u32_value_hashmap_insert(gVromToObjId, gObjectTable[i].vromStart, i);
        }
    }
}

void *loadObjectFromVrom(uintptr_t vromAddr, size_t size) {
    void *obj = recomp_alloc(size);

    DmaMgr_RequestSync(obj, vromAddr, size);

    return obj;
}

RECOMP_EXPORT void *ZGlobalObj_getGlobalObject(ObjectId id) {
    if (id > OBJECT_ID_MAX) {
        return NULL;
    }

    if (!gObjIdToMemTable[id]) {
        uintptr_t vromStart = gObjectTable[id].vromStart;

        size_t size = gObjectTable[id].vromEnd - vromStart;

        gObjIdToMemTable[id] = loadObjectFromVrom(vromStart, size);

        gRepointTracker[id] = recomputil_create_u32_hashset();
    }

    return gObjIdToMemTable[id];
}

RECOMP_EXPORT int ZGlobalObj_getObjIdFromVrom(uintptr_t vromStart, unsigned long *out) {
    return recomputil_u32_value_hashmap_get(gVromToObjId, vromStart, out);
}

RECOMP_EXPORT void *ZGlobalObj_getGlobalObjectFromVrom(uintptr_t vromStart) {
    unsigned long id;

    if (ZGlobalObj_getObjIdFromVrom(vromStart, &id)) {
        return ZGlobalObj_getGlobalObject(id);
    }

    return NULL;
}

RECOMP_EXPORT Gfx *ZGlobalObj_getGlobalGfxPtr(ObjectId id, Gfx *segmentedPtr) {
    if (!isSegmentedPtr(segmentedPtr)) {
        return NULL;
    }

    void *obj = ZGlobalObj_getGlobalObject(id);

    if (!obj) {
        return NULL;
    }

    if (recomputil_u32_hashset_insert(gRepointTracker[id], (uintptr_t)segmentedPtr)) {
        ZGlobalObj_globalizeDL(obj, segmentedPtr);
    }

    return TO_GLOBAL_PTR(obj, segmentedPtr);
}

RECOMP_DECLARE_EVENT(ZGlobalObj_onInit());

RECOMP_CALLBACK("*", recomp_on_init)
void initObjectManagerOnce(PlayState *play) {
    initObjectManager();
    ZGlobalObj_onInit();
}
