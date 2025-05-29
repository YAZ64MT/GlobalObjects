#include "helpers.h"
#include "global.h"

bool isSegmentedPtr(void *p) {
    return SEGMENT_NUMBER(p) <= 0xF;
}
