#ifndef _ZCL_LIST_H
#define _ZCL_LIST_H

#include "ZComDef.h"

struct zcl_item_t {
  uint8 index;
  uint8 *zcl_bytes;
  uint8 length;
  struct zcl_item_t * next;
};

extern struct zcl_item_t * zcl_item_malloc(void);
extern struct zcl_item_t * zcl_item_find_tail(struct zcl_item_t *head);
extern struct zcl_item_t * zcl_item_append(struct zcl_item_t * head, uint8 index, uint8 * zcl_bytes, uint8 length);
extern struct zcl_item_t * zcl_item_get(struct zcl_item_t *head, uint8 index);
extern void zcl_item_free_all(struct item_t *head);

#endif
