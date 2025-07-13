#include "global.h"
#include "recompdata.h"
#include "recomputils.h"
#include "repoint.h"
#include "objectmanager.h"
#include "helpers.h"

// Maps VROM addresses to their respective object IDs
U32ValueHashmapHandle gVromToObjId = 0;

// maps object ids to their location in memory
void *gObjIdToMemTable[OBJECT_ID_MAX] = {0};

// avoid going repointing each dl more than once
U32HashsetHandle gRepointTracker[OBJECT_ID_MAX];

bool isObjectManagerReady(const char *funcName) {
    if (!gVromToObjId) {
        recomp_printf("GlobalObjects: WARNING! FUNCTION %s CALLED BUT OBJECT MANAGER IS NOT INITIALIZED\n", funcName);
        return false;
    }

    return true;
}

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

RECOMP_EXPORT void *GlobalObjects_getGlobalObject(ObjectId id) {
    if (!isObjectManagerReady("GlobalObjects_getGlobalObject")) {
        return NULL;
    }

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

RECOMP_EXPORT bool GlobalObjects_getObjectIdFromVrom(uintptr_t vromStart, ObjectId *out) {
    if (!isObjectManagerReady("GlobalObjects_getObjectIdFromVrom")) {
        return false;
    }
    return recomputil_u32_value_hashmap_get(gVromToObjId, vromStart, out);
}

RECOMP_EXPORT void *GlobalObjects_getGlobalObjectFromVrom(uintptr_t vromStart) {
    if (!isObjectManagerReady("GlobalObjects_getGlobalObjectFromVrom")) {
        return NULL;
    }

    ObjectId id;

    if (GlobalObjects_getObjectIdFromVrom(vromStart, &id)) {
        return GlobalObjects_getGlobalObject(id);
    }

    return NULL;
}

bool hasFieldKeepDependency(ObjectId id) {
    return id == OBJECT_HANA || id == OBJECT_WOOD02;
}

bool hasDangeonKeepDependency(ObjectId id) {
    return id == OBJECT_BDOOR || id == OBJECT_SYOKUDAI;
}

RECOMP_EXPORT Gfx *GlobalObjects_getGlobalGfxPtr(ObjectId id, Gfx *segmentedPtr) {
    if (!isObjectManagerReady("GlobalObjects_getGlobalGfxPtr")) {
        return NULL;
    }

    if (!isSegmentedPtr(segmentedPtr)) {
        return NULL;
    }

    void *obj = GlobalObjects_getGlobalObject(id);

    if (!obj) {
        return NULL;
    }

    if (recomputil_u32_hashset_insert(gRepointTracker[id], (uintptr_t)segmentedPtr)) {
        // workaround for gameplay_dangeon_keep and gameplay_field_keep sharing a segment
        if (hasFieldKeepDependency(id)) {
            Repoint_setFieldOrDangeonKeep(GAMEPLAY_FIELD_KEEP);
        } else if (hasDangeonKeepDependency(id)) {
            Repoint_setFieldOrDangeonKeep(GAMEPLAY_DANGEON_KEEP);
        }

        GlobalObjects_globalizeSegmentedDL(obj, segmentedPtr);

        Repoint_unsetFieldOrDangeonKeep();
    }

    return TO_GLOBAL_PTR(obj, segmentedPtr);
}

// Can't start loading objects in until the dma manager is initialized
RECOMP_DECLARE_EVENT(onReady());

RECOMP_HOOK_RETURN("Main_Init")
void initializeObjectManagerOnce() {
    initObjectManager();
    onReady();
}
