#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "constants.h"
#include "configuration.h"
#include "socket.h"

static int server_sd;
static int client_sd;

static void _data_converter(board_t* board, struct sock_data buffer, uint8_t* white_checkers, uint8_t* black_checkers) {
  const uint32_t b_mask = 0b00000000000000000000000000100000;
  const uint32_t w_mask = 0b00000000000000000000000001000000;
  const uint32_t k_mask = 0b00000000000000000000000011000000;

  *white_checkers = 0;
  *black_checkers = 0;

  memset(board, 0, sizeof(board_t));

  for (int i = 0; i < 9; i++) {
      for (int k = 0; k < 9; k++) {
	  switch (buffer.board[i][k]) {
	    case 'B':
	      board->original.rows[i] = board->original.rows[i] | (b_mask << (k * 3));
	      board->transposed.columns[k] = board->transposed.columns[k] | (b_mask << (i * 3));
	      (*black_checkers)++;
	      break;

	    case 'W':
	      board->original.rows[i] = board->original.rows[i] | (w_mask << (k * 3));
	      board->transposed.columns[k] = board->transposed.columns[k] | (w_mask << (i * 3));
	      (*white_checkers)++;
	      break;

	    case 'K':
	      board->original.rows[i] = board->original.rows[i] | (k_mask << (k * 3));
	      board->transposed.columns[k] = board->transposed.columns[k] | (k_mask << (i * 3));
	      break;
	  }
      }
  }
}

static void _abort_connection() {
  close(client_sd);
  close(server_sd);
}

int connection_init(char* path) {
  size_t size;
  socklen_t client_size;
  struct sockaddr_un sock;
  struct sockaddr client_sock;

  unlink(path);

  server_sd = socket (PF_LOCAL, SOCK_STREAM, 0);
  sock.sun_family = AF_LOCAL;
  strncpy(sock.sun_path, path, sizeof(sock.sun_path));

  size = SUN_LEN(&sock);
  bind(server_sd, (struct sockaddr *) &sock, size);

  listen(server_sd, 1);

  client_sd = accept(server_sd, &client_sock, &client_size);

  return 0;
}


int read_data(board_t* board, color_t* color, uint8_t* white_checkers, uint8_t* black_checkers) {
  struct sock_data buffer;
  ssize_t bytes_read;
  ssize_t size = 0;

  bytes_read = read(client_sd, (void *) (&buffer), sizeof(struct sock_data));
  size += bytes_read;

  while (bytes_read > 0 && size < sizeof(struct sock_data)) {
      bytes_read = read(client_sd, (void *) (&buffer + size), sizeof(struct sock_data));
      size += bytes_read;
  }

  if (size != sizeof(struct sock_data)) {
      _abort_connection();

      return buffer.board[0][0] == 'K' ? 1 : -1;
  } else {
      _data_converter(board, buffer, white_checkers, black_checkers);

      *color = buffer.color == 'W' ? W : B;
      return 0;
  }
}

int write_result(move_t move, move_info_t info) {
  char res[4];

  if (info.isCol) {
      res[0] = res[2] = 97 + info.index;
      res[1] = 49 + move.from;
      res[3] = 49 + move.to;
  } else {
      res[1] = res[3] = 49 + info.index;
      res[0] = 97 + move.from;
      res[2] = 97 + move.to;
  }

  write(client_sd, res, 4);

  return 1;
}
