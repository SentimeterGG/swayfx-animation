#include <stdlib.h>
#include "sway/commands.h"

struct cmd_results *cmd_window_open_duration_ms(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "window_open_duration_ms",
			EXPECTED_AT_LEAST, 1))) {
		return error;
	}

	char *err;
	float val = strtof(argc == 1 ? argv[0] : argv[1], &err);
	if (*err) {
		return cmd_results_new(CMD_INVALID,
			"window_open_duration_ms float invalid");
	}

	if (val < 0 || val > 5000) {
		return cmd_results_new(CMD_FAILURE,
			"window_open_duration_ms value out of bounds");
	}

	config->window_open_duration_ms = val;

	return cmd_results_new(CMD_SUCCESS, NULL);
}
