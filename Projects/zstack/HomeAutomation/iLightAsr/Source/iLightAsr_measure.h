#ifndef _TASK_ASR_H
#define _TASK_ASR_H

#include "ZComDef.h"

// defines
// -----------------------------
#define ASR_MSG_BEFORE_INIT             1
#define ASR_MSG_RUN                     2
#define ASR_MSG_FIND                    3
#define ASR_MSG_ERROR                   4

// types
// -----------------------------
typedef void (*asr_msg_callback_t)(uint8 message, uint8 status);

// functions
// -----------------------------
extern void asr_init(uint8);
extern uint16 asr_event_loop(uint8, uint16);

extern void asr_clear_all_items(void);
extern uint8 asr_add_item(unsigned char index, unsigned char * strp);
extern uint8 asr_flush_items(void);

extern void asr_register_msg_callback(asr_msg_callback_t callback);

#endif