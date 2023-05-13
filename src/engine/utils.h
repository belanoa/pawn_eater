#ifndef UTILS
#define UTILS

#include "constants.h"
#include "types.h"

void row_move_finder(move_t* const move_fifo, uint8_t* const fifo_size, uint32_t row, uint8_t index, color_t color);

void find_all_moves(board_t board, move_t* const restrict move_fifo, move_info_t* const restrict move_info, uint8_t* const size, color_t color);

void generate_board(board_t board, board_t* const new_board, move_t move, move_info_t info, uint8_t* const restrict checkers, color_t color, uint8_t* const restrict checkmate_flag);

#endif
