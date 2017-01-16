#ifndef _ILIGHT_ASR_ITEM_LIST_H_
#define _ILIGHT_ASR_ITEM_LIST_H_

struct item_t {
  unsigned char index;
  unsigned char *strp;
  struct item_t * next;
};

extern struct item_t * item_malloc(void);
extern struct item_t * item_set(struct item_t * head, unsigned char index, unsigned char * strp);
extern struct item_t * item_find_tail(struct item_t *head);
extern struct item_t * item_append(struct item_t * head, unsigned char index, unsigned char * strp);
extern void item_free_all(struct item_t *head);

#endif
