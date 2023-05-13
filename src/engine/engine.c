#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/limits.h>
#include <string.h>

#include "types.h"
#include "utils.h"
#include "constants.h"
#include "socket.h"
#include "routines.h"
#include "configuration.h"
#include "parser.h"

int main(int argc, char** argv) {
  color_t color;
  uint8_t size, black_checkers, white_checkers, global_best_index;
  board_t board;
  arguments_t args [NUM_THREADS];

  pthread_t threads [NUM_THREADS];

  move_t move_fifo [136 + NUM_THREADS];
  move_info_t move_info [136 + NUM_THREADS];

  char sock_path [PATH_MAX];
  int timeout;

  if (parser(argc, argv, sock_path, &timeout)) {
  	printf("USAGE: %s [ OPTIONS ]\nOPTIONS := { -p[ath] | -t[imeout] }\n", argv[0]);
	exit(-1);
  }

  printf("Establishing connection...\n");
  connection_init(sock_path);
  printf("Connection established\n");

  rwlock_init();

  for (int i = 0; i < NUM_THREADS; i++) {
      args[i].move_fifo		= move_fifo;
      args[i].move_info		= move_info;
      args[i].global_best_index	= &global_best_index;
  }

  while (1) {
      memset(move_fifo, 0, sizeof(move_t) * (136 + NUM_THREADS));
      memset(move_info, 0, sizeof(move_info_t) * (136 + NUM_THREADS));
      size = 0;

      printf("Reading data...\n");

      if (read_data(&board, &color, &white_checkers, &black_checkers)) {
          break;
      }

      best_reset(color == W ? 0 : 255);

      printf("Data read: COLOR: %d W: %d B: %d\n", color, white_checkers, black_checkers);

      find_all_moves(board, move_fifo, move_info, &size, color);

      printf("FOUND %d MOVES\n", size);

      for (int i = 0; i < NUM_THREADS; i++) {
	  printf("Creating thread %d...\n", i);

	  args[i].color			= color;
	  args[i].offset		= i;
	  args[i].board			= board;
	  args[i].black_checkers	= black_checkers;
	  args[i].white_checkers	= white_checkers;

	  pthread_create(&(threads[i]), NULL, routine, (void *) &(args[i]));
      }

      printf("Waiting...\n");
      sleep(timeout);

      for (int i = 0; i < NUM_THREADS; i++) {
	  pthread_cancel(threads[i]);
	  pthread_join(threads[i], NULL);
      }

      printf("BEST: %u %u (INDEX: %d) INFO: %u %u\n", move_fifo[global_best_index].from, move_fifo[global_best_index].to, global_best_index, move_info[global_best_index].isCol, move_info[global_best_index].index);
      
      write_result(move_fifo[global_best_index], move_info[global_best_index]);
  }
}
