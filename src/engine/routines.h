#ifndef ROUTINES
#define ROUTINES

#include <pthread.h>

#include "constants.h"
#include "types.h"

typedef struct arguments {
  move_t* move_fifo;
  move_info_t* move_info;

  uint8_t* global_best_index;

  uint8_t offset;
  color_t color;

  board_t board;

  uint8_t white_checkers;
  uint8_t black_checkers;
} arguments_t;

void* routine(void* args_p);
void  rwlock_init();
void  best_reset(uint8_t value);

#endif
