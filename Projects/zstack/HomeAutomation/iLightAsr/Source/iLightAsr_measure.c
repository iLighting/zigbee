#include "ZComDef.h"
#include "OSAL.h"

#include "iLightAsr_ld3320.h"
#include "iLightAsr_item_list.h"
#include "iLightAsr_measure.h"

#include "string.h"
#include "stdio.h"

// defines
// --------------------------------------------------
#define ASR_EVT_INIT              (1<<1)
#define ASR_EVT_ADD_ITEM          (1<<2)
#define ASR_EVT_RUN               (1<<3)
#define ASR_EVT_FIND              (1<<4)
#define ASR_EVT_ERROR             (1<<5)
#define ASR_EVT_POLL              (1<<6)

// interval time in ms
#define ASR_POLL_INTERVAL   50

// global variable
// --------------------------------------------------
uint8 asr_task_id = 0;
uint8 next_state = LD3320_STATE_ASR_FREE;
uint8 next_events = 0;
struct item_t * item_head = NULL;
static asr_msg_callback_t asr_msg_callback = NULL;

#define next(a,b)     do { next_state=(a); next_events=(b); }while(0)

static uint8 set_default_items(struct item_t **head_p) {
	item_set(*head_p, 1, "kai deng");
	item_append(*head_p, 2, "guan deng");
  return 1;
}

void asr_init(uint8 task_id) {
  asr_task_id = task_id;
  osal_set_event(asr_task_id, ASR_EVT_INIT);
}

static void asr_dispatch_one_msg(uint8 msg, uint8 status) {
  if (asr_msg_callback) {
    asr_msg_callback(msg, status);
  }
}

void asr_register_msg_callback(asr_msg_callback_t callback) {
  asr_msg_callback = callback;
}

uint16 asr_event_loop(uint8 task_id, uint16 events) {
	// ASR_EVT_ERROR -> ASR_EVT_INIT
  if (events & ASR_EVT_ERROR) {
    asr_dispatch_one_msg(ASR_MSG_ERROR, 0);
    osal_start_timerEx(task_id, ASR_EVT_INIT, 100);
    return events ^ ASR_EVT_ERROR;
  }

	// ASR_EVT_INIT -> ASR_EVT_ADD_ITEM
  if (events & ASR_EVT_INIT) {
    asr_dispatch_one_msg(ASR_MSG_BEFORE_INIT, 0);
    if (ld3320_init(LD3320_INIT_MODE_ASR)) {
      if (item_head != NULL) {
        item_free_all(item_head);
        item_head = NULL;
      }
      item_head = item_malloc();
      if (item_head==NULL) {
        osal_start_timerEx(task_id, ASR_EVT_INIT, 100);
      }
			// set default items
      set_default_items(&item_head);
			osal_set_event(task_id, ASR_EVT_ADD_ITEM);
    } else {
      osal_start_timerEx(task_id, ASR_EVT_INIT, 100);
    }
    return events ^ ASR_EVT_INIT;
  }

	// ASR_EVT_ADD_ITEM -> ASR_EVT_RUN
  if (events & ASR_EVT_ADD_ITEM) {
    struct item_t *p = item_head;
		if (ld3320_get_state() == LD3320_STATE_ASR_FREE) {
			while (p!=NULL) {
				ld3320_add_item(p->index, p->strp);
				p = p->next;
			}
			osal_set_event(task_id, ASR_EVT_RUN);
		} else {
			osal_start_timerEx(task_id,ASR_EVT_ADD_ITEM,30);
		}
    return events ^ ASR_EVT_ADD_ITEM;
  }

	// ASR_EVT_RUN -> ASR_EVT_POLL
  if (events & ASR_EVT_RUN) {
    if (ld3320_asr_run()) {
      asr_dispatch_one_msg(ASR_MSG_RUN, 0);
			osal_set_event(task_id, ASR_EVT_POLL);
    } else {
      osal_start_timerEx(task_id, ASR_EVT_INIT, 100);
    }
    return events ^ ASR_EVT_RUN;
  }

	// ASR_EVT_FIND -> ASR_EVT_INIT
  if (events & ASR_EVT_FIND) {
    asr_dispatch_one_msg(ASR_MSG_FIND, ld3320_get_asr_result(0));
    osal_set_event(task_id, ASR_EVT_INIT);
    return events ^ ASR_EVT_FIND;
  }

	// ASR_EVT_POLL -> ASR_EVT_ERROR, ASR_EVT_FIND, ASR_EVT_RUN, ASR_EVT_POLL
	if (events & ASR_EVT_POLL) {
		uint8 state = ld3320_get_state();
		switch (state) {
			case LD3320_STATE_ASR_ERROR:
				osal_set_event(task_id, ASR_EVT_ERROR);
				break;
			case LD3320_STATE_ASR_FIND:
				osal_set_event(task_id, ASR_EVT_FIND);
        break;
			case LD3320_STATE_ASR_FREE:
				osal_set_event(task_id, ASR_EVT_INIT);
				break;
			case LD3320_STATE_ASR_BUSY:
			default:
				osal_start_timerEx(asr_task_id, ASR_EVT_POLL, ASR_POLL_INTERVAL);
		}
		return events ^ ASR_EVT_POLL;
	}

  return 0;
}

void asr_clear_all_items(void) {
  struct item_t * this = item_head;
  struct item_t * next = NULL;
  while(this!=NULL) {
    next = this->next;
    osal_mem_free(this->strp);
    osal_mem_free(this);
    this = next;
  }
}

uint8 asr_add_item(unsigned char index, unsigned char * strp) {
  if(item_append(item_head, index, strp)) {
    return 1;
  }
  return 0;
}

uint8 asr_flush_items(void) {
  // 重启ld3320以清空旧的命令字
  osal_stop_timerEx(asr_task_id, ASR_EVT_INIT);
	osal_stop_timerEx(asr_task_id, ASR_EVT_ERROR);
	osal_stop_timerEx(asr_task_id, ASR_EVT_ADD_ITEM);
	osal_stop_timerEx(asr_task_id, ASR_EVT_RUN);
	osal_stop_timerEx(asr_task_id, ASR_EVT_FIND);
  osal_stop_timerEx(asr_task_id, ASR_EVT_POLL);
  osal_set_event(asr_task_id, ASR_EVT_INIT);
  return 1;
}
