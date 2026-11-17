#ifndef REVISION_DATA_H
#define REVISION_DATA_H
#include "engine.h"
typedef struct {const char *title,*method,*example; int diagram,tool;} Card;
typedef struct {const char *name; const Card *cards; int count;} Topic;
enum {DIAG_NONE, DIAG_MOD, DIAG_CAST, DIAG_INTERSECT, DIAG_AREA, DIAG_TURN, DIAG_RANGE, DIAG_VECTOR};
#define REVISION_TOPIC_COUNT 16
extern const Topic revision_topics[REVISION_TOPIC_COUNT];
#endif
