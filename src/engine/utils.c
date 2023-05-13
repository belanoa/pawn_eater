#include "utils.h"

static void _move_finder(move_t* const move_fifo, uint8_t* const fifo_size, uint32_t allies, uint32_t obstacles) {

  uint8_t shift_count	= 8;
  uint8_t fifo_p_to	= *fifo_size;
  uint8_t fifo_p_from	= *fifo_size;
  uint8_t left_counter;

  while (allies != 0) {
      if (allies > obstacles) {	//The first checker is an ally
	  while (allies < 0b01000000000000000000000000000000) {
	      move_fifo [fifo_p_to].to = shift_count;
	      shift_count--;
	      fifo_p_to++;
	      allies = allies << 3;
	      obstacles = obstacles << 3;
	  }

	  allies = allies << 3;
	  obstacles = obstacles << 3;

	  while (fifo_p_from < fifo_p_to) {
	      move_fifo [fifo_p_from].from = shift_count;
	      fifo_p_from++;
	  }

	  while (allies > obstacles) {
	      left_counter = 0;

	      while (obstacles < 0b00100000000000000000000000000000) {
		  move_fifo [fifo_p_to].to = shift_count - 1;	//Checker on the left
		  move_fifo [fifo_p_to + 1].to = shift_count - 1;	//Checker on the right
		  fifo_p_from += 2;
		  fifo_p_to += 2;
		  shift_count--;
		  left_counter++;
		  move_fifo [fifo_p_from - 1].from = shift_count + left_counter;
		  allies = allies << 3;
		  obstacles = obstacles << 3;
	      }

	      allies = allies << 3;
	      obstacles = obstacles << 3;
	      shift_count--;

	      //Fill from for the checker on the left
	      for (int i = 2; i <= left_counter * 2; i += 2) {
		  move_fifo [fifo_p_from - i].from = shift_count;
	      }
	  }

	  left_counter = 0;

	  while (obstacles < 0b00100000000000000000000000000000 && shift_count != 0) {
	      move_fifo [fifo_p_to].to = shift_count - 1;
	      move_fifo [fifo_p_from].from = shift_count + left_counter;
	      shift_count--;
	      left_counter++;
	      allies = allies << 3;
	      obstacles = obstacles << 3;
	      fifo_p_from++;
	      fifo_p_to++;
	  }

	  shift_count--;
      } else {	//The first checker is a obstacle, consume until the first one is an ally
	  do {
	      shift_count--;
	      allies = allies << 3;
	      obstacles = obstacles << 3;
	  } while (obstacles > allies);
      }
  }

  *fifo_size = fifo_p_to;
}

void row_move_finder(move_t* const move_fifo, uint8_t* const fifo_size, const uint32_t row, const uint8_t index, const color_t color) {
  uint32_t allies;
  uint32_t obstacles;

  if (color == W) {
      allies = row & WHITE_MASK;
      obstacles = (row & BLACK_MASK) | (allies >> 1);
  } else {
      allies = (row & BLACK_MASK) << 1;
      obstacles = ((row & WHITE_MASK) >> 1) | (allies >> 1);
  }

  switch (index) {
    case 0:
    case 8:

      if (color == B) {
	  _move_finder(move_fifo, fifo_size, allies & (BOARD_MASK_0 << 1), obstacles);	//Also find moves of the 3 central checkers
	  allies = allies ^ ((BOARD_MASK_0 << 1) & allies);
      }

      obstacles = obstacles | BOARD_MASK_0;
      break;

    case 1:
    case 7:
      obstacles = obstacles | BOARD_MASK_1;
      break;

    case 3:
    case 5:
      obstacles = obstacles | BOARD_MASK_2;
      break;

    case 4:

      if (color == B) {
	  _move_finder(move_fifo, fifo_size, allies & (BOARD_MASK_3 << 1), obstacles | 0b00000000000000100000000000000000);	//Find the moves of the outer checkers
	  allies = allies ^ ((BOARD_MASK_3 << 1) & allies);
      }

      obstacles = obstacles | BOARD_MASK_3;
      break;
  }

  _move_finder(move_fifo, fifo_size, allies, obstacles);
}

void find_all_moves(board_t board, move_t* const restrict move_fifo, move_info_t* const restrict move_info, uint8_t* const size, const color_t color) {
  uint32_t* restrict board_rows;
  uint8_t info_pointer = 0;

  for (int j = 0; j < 2; j++) {
      if (j == 0) {
	  board_rows = board.original.rows;
      } else {
	  board_rows = board.transposed.columns;
      }

      for (int i = 0; i < 9; i++) {
	  row_move_finder(move_fifo, size, board_rows[i], i, color);

	  while (info_pointer < *size) {
	      move_info[info_pointer].index = i;
	      move_info[info_pointer].isCol = j;
	      info_pointer++;
	  }
      }
  }
}

void generate_board(board_t board, board_t* const new_board, const move_t move, const move_info_t info, uint8_t* const restrict checkers, const color_t color, uint8_t* const restrict checkmate_flag) {
  uint8_t king_surrounded	= 0;
  uint8_t king_dir		= 0;

  *checkmate_flag = 0;

  uint32_t* restrict board_rows;
  uint32_t* restrict board_columns;
  uint32_t* restrict new_board_rows;
  uint32_t* restrict new_board_columns;

  if (info.isCol) {
      board_rows	= board.transposed.columns;
      board_columns	= board.original.rows;
      new_board_rows	= new_board->transposed.columns;
      new_board_columns	= new_board->original.rows;
  } else {
      board_rows	= board.original.rows;
      board_columns	= board.transposed.columns;
      new_board_rows	= new_board->original.rows;
      new_board_columns	= new_board->transposed.columns;
  }

  uint32_t target = (board_rows[info.index] >> (move.from * 3)) & REMOVER_MASK;

  if (color == W && target == C_KING && ((move.to == 8) | (move.to == 0))) {	//Goal check white
      *checkmate_flag = 1;
      return;
  }

  new_board_rows[info.index]	= (board_rows[info.index] ^ (target << (move.from * 3))) | (target << (move.to * 3));
  new_board_columns[move.from]	= board_columns[move.from] ^ (target << (info.index * 3));
  new_board_columns[move.to]	= board_columns[move.to] ^ (target << (info.index * 3));

  //Check for changes in the board
  switch (move.to) {
    case 0:
    case 8:
      target = new_board_columns[move.to] | OBSTACLE_MASK_0;
      break;

    case 1:
    case 7:
      target = new_board_columns[move.to] | OBSTACLE_MASK_1;
      break;

    case 3:
    case 5:
      target = new_board_columns[move.to] | OBSTACLE_MASK_2;
      break;

    case 4:
      target = new_board_columns[move.to] | OBSTACLE_MASK_3;
      break;

    default:
      target = new_board_columns[move.to];
  }

  if (color == W) {
      //Capture check column left
      if (((target >> (info.index * 3)) & CAPTURE_MASK_W) == 0b00000000000000000000000001000101) {
	  (*checkers)--;
	  new_board_columns[move.to] = new_board_columns[move.to] ^ (REMOVE_BLACK << (info.index * 3 - 3));
	  new_board_rows[info.index - 1] = new_board_rows[info.index - 1] ^ (REMOVE_BLACK << (move.to * 3));
      }

      //Capture check column right
      if (((target >> (info.index * 3 + 6)) & CAPTURE_MASK_W) == 0b00000000000000000000000001000101) {
	  (*checkers)--;
	  new_board_columns[move.to] = new_board_columns[move.to] ^ (REMOVE_BLACK << (info.index * 3 + 3));
	  new_board_rows[info.index + 1] = new_board_rows[info.index + 1] ^ (REMOVE_BLACK << (move.to * 3));
      }
  } else {
      target = target << 1;

      switch ((target >> (info.index * 3)) & CAPTURE_MASK_B) {
	case 0b00000000000000000000000001010001:
	  (*checkers)--;
	  new_board_columns[move.to] = new_board_columns[move.to] ^ (REMOVE_WHITE << (info.index * 3 - 3));
	  new_board_rows[info.index - 1] = new_board_rows[info.index - 1] ^ (REMOVE_WHITE << (move.to * 3));
	  break;

	case 0b00000000000000000000000001110001:
	  king_surrounded 	= 1;
	  king_dir 		= 1;
	  break;
      }

      switch ((target >> (info.index * 3 + 6)) & CAPTURE_MASK_B) {
	case 0b00000000000000000000000001010001:
	  (*checkers)--;
	  new_board_columns[move.to] = new_board_columns[move.to] ^ (REMOVE_WHITE << (info.index * 3 + 3));
	  new_board_rows[info.index + 1] = new_board_rows[info.index + 1] ^ (REMOVE_WHITE << (move.to * 3));
	  break;

	case 0b00000000000000000000000001110001:
	  king_surrounded 	= 1;
	  king_dir 		= 2;
	  break;
      }
  }

  switch (info.index) {
    case 0:
    case 8:
      target = new_board_rows[info.index] | OBSTACLE_MASK_0;
      break;

    case 1:
    case 7:
      target = new_board_rows[info.index] | OBSTACLE_MASK_1;
      break;

    case 3:
    case 5:
      target = new_board_rows[info.index] | OBSTACLE_MASK_2;
      break;

    case 4:
      target = new_board_rows[info.index] | OBSTACLE_MASK_3;
      break;

    default:
      target = new_board_rows[info.index];
  }

  if (color == W) {
      if (move.to < move.from) {
	  if (((target >> (move.to * 3)) & CAPTURE_MASK_W) == 0b00000000000000000000000001000101) {
	      (*checkers)--;
	      new_board_rows[info.index] = new_board_rows[info.index] ^ (REMOVE_BLACK << (move.to * 3 - 3));
	      new_board_columns[move.to - 1] = new_board_columns[move.to - 1] ^ (REMOVE_BLACK << (info.index * 3));
	  }
      } else {
	  if (((target >> ((move.to * 3) + 6)) & CAPTURE_MASK_W) == 0b00000000000000000000000001000101) {
	      (*checkers)--;
	      new_board_rows[info.index] = new_board_rows[info.index] ^ (REMOVE_BLACK << (move.to * 3 + 3));
	      new_board_columns[move.to + 1] = new_board_columns[move.to + 1] ^ (REMOVE_BLACK << (info.index * 3));
	  }
      }
  } else {
      target = target << 1;

      if (move.to < move.from) {
	  switch ((target >> (move.to * 3)) & CAPTURE_MASK_B) {
	    case 0b00000000000000000000000001010001:
	      (*checkers)--;
	      new_board_rows[info.index] = new_board_rows[info.index] ^ (REMOVE_BLACK << (move.to * 3 - 3));
	      new_board_columns[move.to - 1] = new_board_columns[move.to - 1] ^ (REMOVE_BLACK << (info.index * 3));
	      break;

	    case 0b00000000000000000000000001110001:
	      king_surrounded 	= 1;
	      king_dir 		= 3;
	      break;
	  }
      } else {
	  switch ((target >> ((move.to * 3) + 6)) & CAPTURE_MASK_B) {
	    case 0b00000000000000000000000001010001:
	      (*checkers)--;
	      new_board_rows[info.index] = new_board_rows[info.index] ^ (REMOVE_BLACK << (move.to * 3 + 3));
	      new_board_columns[move.to + 1] = new_board_columns[move.to + 1] ^ (REMOVE_BLACK << (info.index * 3));
	      break;

	    case 0b00000000000000000000000001110001:
	      king_surrounded 	= 1;
	      king_dir 		= 4;
	      break;
	  }
      }
  }
  
  uint8_t king_pos;
  uint32_t king_row;
  uint8_t shift;

  if (color == B && king_surrounded == 1) {	//Goal check black
      switch (king_dir) {
	case 1:	//UP
	  king_row = new_board_rows[info.index - 1] ;
	  king_pos = info.index - 1;
	  shift = move.to - 1;
	  break;

	case 2:	//DOWN
	  king_row = new_board_rows[info.index + 1];
	  king_pos = info.index + 1;
	  shift = move.to - 1;
	  break;

	case 3:	//RIGHT
	  king_row = new_board_columns[move.to - 1];
	  king_pos = move.to - 1;
	  shift = info.index - 1;
	  break;

	case 4:	//LEFT
	  king_row = new_board_columns[move.to + 1];
	  king_pos = move.to + 1;
	  shift = info.index - 1;
	  break;
      }

      //If the king is not adjacent to or on the throne it just needs to be surrounded on 2 sides
      if (!((king_pos == 4 && shift >= 2 && shift <= 4) || (shift == 3 && king_pos >= 3 && king_pos <= 5))) {
      	*checkmate_flag = 1;
      } else {
	      switch (king_pos) {
		case 0:                                                 	
		case 8:
		  king_row = king_row | OBSTACLE_MASK_0;
		  break;
								     
		case 1:
		case 7:
		  king_row = king_row | OBSTACLE_MASK_1;
		  break;
								     
		case 3:
		case 5:
		  king_row = king_row | OBSTACLE_MASK_2;
		  break;
								     
		case 4:
		  king_row = king_row | OBSTACLE_MASK_3;
		  break;
	      }

	      king_row = king_row >> (shift * 3 + 5);

	      if ((king_row & CAPTURE_MASK_B) == 0b00000000000000000000000001110001) {
		  *checkmate_flag = 1;
	      }
      }
  } else if (color == W && *checkers == 0) {	//Goal check white
      *checkmate_flag = 1;
  }
}
