#ifndef SOCKET
#define SOCKET
#include "types.h"

#pragma pack(push, 1)
typedef struct sock_data {

  char board[9][9];
  char color;

} sock_data_t;
#pragma pack(pop)

int connection_init();
int read_data(board_t* board, color_t* color, uint8_t* white_checkers, uint8_t* black_checkers);
int write_result(move_t move, move_info_t info);

#endif
