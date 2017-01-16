#include "ZComDef.h"
#include "osal.h"


struct zcl_item_t * zcl_item_malloc(void) {
  struct zcl_item_t * p = NULL;
  p = osal_mem_alloc(sizeof(struct zcl_item_t));
  if(p==NULL)
    return NULL;
  osal_memset(p, sizeof(struct zcl_item_t), 0);
  p->next = NULL;
  return p;
}

struct zcl_item_t * zcl_item_find_tail(struct zcl_item_t *head) {
  struct zcl_item_t * this = head;
  while(this->next!=NULL) {
    this = this->next;
  }
  return this;
}

struct zcl_item_t * zcl_item_append(struct zcl_item_t * head,
                                    uint8 index,
                                    uint8 * zcl_bytes,
                                    uint8 length) {
  struct zcl_item_t * tail = zcl_item_find_tail(head);
  struct zcl_item_t * new_item_p = NULL;
  uint8 * new_bytes = NULL;

  new_item_p = zcl_item_malloc();
  if(new_item_p==NULL)
    return NULL;

  new_bytes = osal_mem_alloc(length);
  if (new_bytes==NULL) {
    osal_mem_free(new_item_p);
    return NULL;
  }

  new_item_p->index = index;
  new_item_p->zcl_bytes = new_bytes;
  new_item_p->length = length;
  tail->next = new_item_p;
  return new_item_p;
}

struct zcl_item_t * zcl_item_get(struct zcl_item_t *head, uint8 index) {
  struct zcl_item_t * this = head;
  while(this->next!=NULL) {
    if (this->index == index)
      break;
    this = this->next;
  }
  return this;
}

void zcl_item_free_all(struct zcl_item_t *head) {
  struct zcl_item_t *this = head;
  struct zcl_item_t *next = NULL;
  while(this!=NULL) {
    next = this->next;
    osal_mem_free(this->zcl_bytes);
    osal_mem_free(this);
    this = next;
  }
}
