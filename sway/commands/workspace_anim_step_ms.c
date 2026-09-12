#include <errno.h>
#include <stdlib.h>
#include "sway/commands.h"

struct cmd_results *cmd_workspace_anim_step_ms(int argc, char **argv) {
	struct cmd_results *error = NULL;
	if ((error = checkarg(argc, "workspace_anim_step_ms", EXPECTED_AT_LEAST, 1))) {
		return error;
	}

	char *err;
	float val = strtof(argv[0], &err);
	if (*err) {
		return cmd_results_new(CMD_INVALID, "workspace_anim_step_ms float invalid");
	}

	if (val < 0 || val > 5000) {
		return cmd_results_new(CMD_FAILURE, "workspace_anim_step_ms value out of bounds");
	}

	config->workspace_anim_step_ms = val;

	return cmd_results_new(CMD_SUCCESS, NULL);
}
