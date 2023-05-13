#ifndef TYPES
#define TYPES

#include <stdint.h>

typedef struct board {

  union {

    struct {
      struct {
	uint32_t rows [9];
      } original;

      struct {
	uint32_t columns [9];
      } transposed;
    };

    uint32_t elements[18];

  };

} board_t;

typedef struct move {
    unsigned from:4;
    unsigned to:4;
} move_t;

typedef struct move_info  {

  unsigned isCol : 4;
  unsigned index : 4;

} move_info_t;

typedef enum colors {W, B} color_t;

#endif
