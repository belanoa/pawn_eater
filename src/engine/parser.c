#include <getopt.h>
#include <linux/limits.h>
#include <string.h>
#include <stdlib.h>

#include "parser.h"
#include "configuration.h"

int parser(int argc, char* const * argv, char* const sock_path, int* const timeout) {
	static struct option long_options[] = {
		{"path", optional_argument, NULL, 'p'},
		{"timeout", optional_argument, NULL, 't'},
		{NULL, 0, NULL, 0}
	};

	int val;

	sock_path[0] = '\0';
	*timeout = 0;

	while ((val = getopt_long(argc, argv, "p:t:", long_options, NULL)) != -1) {
		switch (val) {
			case 'p':
				strncpy(sock_path, optarg, PATH_MAX);
				break;

			case 't':
				*timeout = atoi(optarg);
				if (*timeout <= 0) {
					return -1;
				}
				break;

			default:
				return -1;	
		}
	}

	if (sock_path[0] == '\0') {
		strncpy(sock_path, SOCK_PATH, PATH_MAX);
	}

	if (*timeout == 0) {
		*timeout = DEFAULT_TIMEOUT;
	}

	return 0;
}
