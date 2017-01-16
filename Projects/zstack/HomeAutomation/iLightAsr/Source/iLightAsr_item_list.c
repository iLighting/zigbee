#include "iLightAsr_item_list.h"
#include "osal.h"
#include "string.h"

#ifndef NULL
#define NULL 0
#endif

#define specific_mem_malloc(x) osal_mem_alloc(x)
#define specific_mem_free(x) osal_mem_free(x)
#define specific_mem_set(a,b,c) osal_memset((a), (b), (c))
#define specific_mem_cpy(a,b,c) osal_memcpy((a),(b),(c))

struct item_t * item_malloc(void) {
  struct item_t * p = NULL;
  p = specific_mem_malloc(sizeof(struct item_t));
  if(p==NULL)
    return NULL;
	p->index = 0;
	p->strp = NULL;
  p->next = NULL;
  return p;
}

struct item_t * item_find_tail(struct item_t *head) {
  struct item_t * this = head;
  while(this->next!=NULL) {
    this = this->next;
  }
  return this;
}

struct item_t * item_set(struct item_t * node, unsigned char index, unsigned char *strp){
	unsigned char * new_strp = NULL;
	uint8 s_len = strlen((char const *)strp);
	new_strp = (unsigned char *)specific_mem_malloc(s_len+1);
 	if (new_strp==NULL) return NULL;
	specific_mem_cpy(new_strp, strp, s_len+1);
	// free exist strp
	if (node->strp != NULL) specific_mem_free(node->strp);
	node->index = index;
	node->strp = new_strp;
	return node;
}

struct item_t * item_append(struct item_t * head, unsigned char index, unsigned char * strp) {
  struct item_t * tail = item_find_tail(head);
  struct item_t * new_item_p = NULL;

  new_item_p = item_malloc();
  if(new_item_p==NULL) return NULL;

	if (item_set(new_item_p, index, strp)) {
		tail->next = new_item_p;
		return new_item_p;
	} else {
		specific_mem_free(new_item_p);
		return NULL;
	}
}

void item_free_all(struct item_t *head) {
  struct item_t *this = head;
  struct item_t *next = NULL;
  while(this!=NULL) {
    next = this->next;
    specific_mem_free(this->strp);
    specific_mem_free(this);
    this = next;
  }
}
