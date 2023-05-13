#include "evaluator.h"
#include "utils.h"

static uint8_t _heuristics(const board_t board, const uint8_t white_checkers, const uint8_t black_checkers) {
  uint32_t cur, king;
  uint8_t king_pos [2];
  uint8_t free_escapes = 0;
  uint32_t king_rows [2];

  uint8_t res = 127 + white_checkers * 4 - black_checkers * 2;

  for (int j = 0; j < 2; j++) {
      for (int i = 1; i < 8; i++) {
	  cur = j == 0 ? board.original.rows[i] : board.transposed.columns[i];
	  king = cur & KING_MASK;

	  if (king) {
	      king_pos [j] = i;

	      //Row and column check
	      switch (i) {
		case 1:
		case 7:
		  res = res + 2;

		  if (king > (cur >> 1)) {
		      res = res + 8;
		      free_escapes++;
		  }

		  if (cur << ((9 - i) * 3) == 0) {
		      res = res + 8;
		      free_escapes++;
		  }

		  king_rows[j] = cur | BOARD_MASK_1;
		  break;

		case 2:
		case 6:
		  res = res + 4;

		  if (king > (cur >> 1)) {
		      res = res + 8;
		      free_escapes++;
		  }

		  if (cur << ((9 - i) * 3) == 0) {
		      res = res + 8;
		      free_escapes++;
		  }
                  
                  king_rows[j] = cur;
		  break;

		case 3:
		case 5:
                  king_rows[j] = cur | BOARD_MASK_2;  
		  break;

		case 4:
		  res = res - 4;
		  king_rows[j] = cur | BOARD_MASK_3;  
		  break;
	      }

	      break;
	  }
      }
  }

  if (free_escapes >= 2) {
      return 254;
  }

  king_rows[0] = king_rows[0] >> (king_pos[1] * 3);
  king_rows[1] = king_rows[1] >> (king_pos[0] * 3);

  if (king_rows[0] & 0b00000000000000000000000000000100) {
      res -= 4;
  }

  if (king_rows[1] & 0b00000000000000000000000000000100) {  
      res -= 4;
  }                                           
  
  if (king_rows[0] & 0b00000000000000000000000100000000) {
      res -= 4;
  }
                                                         
  if (king_rows[1] & 0b00000000000000000000000100000000) { 
      res -= 4;
  }                                                       

  return res;
}

uint8_t evaluate(const board_t board, const uint8_t depth, const uint8_t maxdepth, const color_t color, const uint8_t white_checkers, const uint8_t black_checkers, uint8_t alpha, uint8_t beta) {
  move_t move_fifo [14];
  move_info_t current_move_info;
  uint8_t fifo_size;
  uint8_t checkmate_flag;
  board_t new_board;
  uint8_t checkers;
  uint8_t eval;

  if (depth == maxdepth) {
        //Evaluate board using heuristics and return the result
        return _heuristics(board, white_checkers, black_checkers);
    } else {
	for (int j = 0; j < 2; j++) {
		current_move_info.isCol = j;

	    for (int i = 0; i < 9; i++) {
		fifo_size = 0;
		current_move_info.index = i;

		row_move_finder(move_fifo, &fifo_size, board.elements[i + 9 * j], i, color);

		for (int k = 0; k < fifo_size; k++) {
		    checkers = color == W ? black_checkers : white_checkers;
		    new_board = board;

		    generate_board(board, &new_board, move_fifo[k], current_move_info, &checkers, color, &checkmate_flag);

		    if (checkmate_flag) {
			return color == W ? 255 : 0;
		    }

		    if (color == W) {
			eval = evaluate(new_board, depth + 1, maxdepth, B, white_checkers, checkers, alpha, beta);

			if (eval >= beta) {
			    return eval;
			} else {
			    alpha = eval > alpha ? eval : alpha;
			}
		    } else {
			eval = evaluate(new_board, depth + 1, maxdepth, W, black_checkers, checkers, alpha, beta);

			if (eval <= alpha) {
			    return eval;
			} else {
			    beta = eval < beta ? eval : beta;
			}
		    }
		}
	    }
	}

	return color == W ? alpha : beta;
    }
}
