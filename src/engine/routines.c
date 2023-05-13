#include "routines.h"
#include "evaluator.h"
#include "utils.h"
#include "configuration.h"

#include <stdlib.h>
#include <pthread.h>
#include <memory.h>

struct cleanup_args {
  uint8_t* global_best_p;
  uint8_t* depth;
  color_t  color;
};

static pthread_rwlock_t rwlock;

static uint8_t best_indexes[255];
static uint8_t global_best[255];
static uint8_t checkmate_depth;

static void cleanup_routine(void* args_p) {
  struct cleanup_args* args = args_p;
  
  uint8_t  depth = *(args->depth);

  //Check if a move better than the current best has been found
  if (args->color == W) {
	if (global_best[depth] >= global_best[depth - 1]) {
		*(args->global_best_p) = best_indexes[depth];
	}
  } else {
 	if (global_best[depth] <= global_best[depth - 1]) {
		*(args->global_best_p) = best_indexes[depth];
	}
  }
}

static void rwlock_unlock(void* args_p) {
  pthread_rwlock_unlock(&rwlock);
}

void rwlock_init() {
  pthread_rwlockattr_t attr;

  pthread_rwlockattr_init(&attr);
  pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);

  pthread_rwlock_init(&rwlock, &attr);
}

void best_reset(uint8_t value) {
  memset(global_best, value, 255);
  checkmate_depth = 255;
}

void* routine(void* args_p) {
  arguments_t* args = (arguments_t *) args_p;

  uint8_t index = args->offset;
  uint8_t depth = 0;

  uint8_t alpha	= 0;
  uint8_t beta	= 255;

  board_t new_board;
  uint8_t checkers;
  uint8_t checkmate_flag;
  uint8_t eval;

  uint8_t* best_index_p = best_indexes;
  uint8_t* global_best_p = global_best;

  struct cleanup_args c_args;
  c_args.global_best_p	= args->global_best_index;
  c_args.color		= args->color;

  pthread_cleanup_push(cleanup_routine, args_p);

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  while (1) {
      while (args->move_fifo[index].from != 0 || args->move_fifo[index].to != 0) {
	  new_board		= args->board;
	  checkers 		= args->color == W ? args->black_checkers : args->white_checkers;
	  checkmate_flag	= 0;

	  //Compute the new board
	  generate_board(args->board, &new_board, args->move_fifo[index], args->move_info[index], &checkers, args->color, &checkmate_flag);

	  if (depth == 0 && checkmate_flag) {
	      pthread_rwlock_wrlock(&rwlock);
	      pthread_cleanup_push(rwlock_unlock, NULL);

	      *global_best_p = args->color == W ? 255 : 0;
	      *(args->global_best_index) = index;
	      checkmate_depth = 0;
		
	      pthread_cleanup_pop(1);

	      return NULL;
	  }

	  if (args->color == W) {
	      pthread_rwlock_rdlock(&rwlock);
	      pthread_cleanup_push(rwlock_unlock, NULL);

	      alpha = *global_best_p;
	
	      pthread_cleanup_pop(1);

	      eval = evaluate(new_board, 0, depth, B, args->white_checkers, checkers, alpha, 255);
	      
	      if (eval == 255) {
		  pthread_rwlock_wrlock(&rwlock);
		  pthread_cleanup_push(rwlock_unlock, NULL);

		  if (*global_best_p != 255 && depth < checkmate_depth) {
		  	*global_best_p = 255;
			*(args->global_best_index) = index;
		  	checkmate_depth = depth;
		  }
			
		  pthread_cleanup_pop(1);

		  return NULL;
	      }

	      if (eval > alpha) {
		  pthread_rwlock_wrlock(&rwlock);
		  pthread_cleanup_push(rwlock_unlock, NULL);

		  if (eval > *global_best_p) {
			  *best_index_p  = index;
			  *global_best_p = eval;
		  }

		  pthread_cleanup_pop(1);
	      }
	  } else {
	      pthread_rwlock_rdlock(&rwlock);
	      pthread_cleanup_push(rwlock_unlock, NULL);

	      beta = *global_best_p;

	      pthread_cleanup_pop(1);

	      eval = evaluate(new_board, 0, depth, W, checkers, args->black_checkers, 0, beta);

	      if (eval == 0) {
		  pthread_rwlock_wrlock(&rwlock);
		  pthread_cleanup_push(rwlock_unlock, NULL);

		  if (*global_best_p != 0 && depth < checkmate_depth) {
		  	*global_best_p = 0;
			*(args->global_best_index) = index;
		  	checkmate_depth = depth;
		  }
		  
		  pthread_cleanup_pop(1);

		  return NULL;
	      }

	      if (eval < beta) {
		  pthread_rwlock_wrlock(&rwlock);
	          pthread_cleanup_push(rwlock_unlock, NULL);

		  if (eval < *global_best_p) {
		 	*best_index_p  = index;
			*global_best_p = eval;                       
		  }

		  pthread_cleanup_pop(1);
	      }
	  }

	  index += NUM_THREADS;
      }

      pthread_rwlock_rdlock(&rwlock);
      pthread_cleanup_push(rwlock_unlock, NULL);

      if (checkmate_depth == 255) {
      	*(args->global_best_index) = *best_index_p;
      }

      pthread_cleanup_pop(1);

      alpha = 0;
      beta = 255;

      index = args->offset;
      depth += 1;

      global_best_p += 1;
      best_index_p += 1;
  }

  pthread_cleanup_pop(0);
}
