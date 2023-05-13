#ifndef EVALUATOR
#define EVALUATOR

#include "types.h"
#include "constants.h"

uint8_t evaluate(board_t board, uint8_t depth, uint8_t maxdepth, color_t color, uint8_t white_checkers, uint8_t black_checkers, uint8_t alpha, uint8_t beta);

#endif
