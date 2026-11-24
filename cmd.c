#include "cmd.h"
#include "config.h"
#include "UART.h"
#include "trigger.h"
#include "capture.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// one reply per command: OK or ERR <code> <reason>
// ERR 1 unknown command
// ERR 2 bad argument
// ERR 3 busy (capture running)
// ERR 4 not implemented yet
// ERR 5 trigger is off

static int read_number(char *s, uint32_t *value) {
	char *end;
	if (s == 0 || *s == 0)
		return 0;
	*value = strtoul(s, &end, 10);
	return (*end == 0);
}

void Cmd_Handle(void) {
	char line[LINE_MAX];
	char *cmd, *arg;
	uint32_t i, n;
	const char *state;

	for (i=0; i<LINE_MAX; i++)
		line[i] = line_buf[i];
	line[LINE_MAX-1] = 0;
	line_ready = 0;

	cmd = strtok(line, " ");
	arg = strtok(0, " ");
	if (cmd == 0)
		return; // empty line

	if (cap_state != CAP_IDLE) {
		printf("ERR 3 busy\n");
		return;
	}

	if (strcmp(cmd, "TRIG") == 0) {
		if (arg == 0)
			n = TRIG_DEFAULT_HZ;
		else if (!read_number(arg, &n) || n < TRIG_MIN_HZ || n > TRIG_MAX_HZ) {
			printf("ERR 2 rate must be %u to %u\n", TRIG_MIN_HZ, TRIG_MAX_HZ);
			return;
		}
		Trigger_Set(n);
		printf("OK\n");
	}
	else if (strcmp(cmd, "TRIGOFF") == 0) {
		Trigger_Off();
		printf("OK\n");
	}
	else if (strcmp(cmd, "CAPTURE") == 0) {
		if (!read_number(arg, &n) || n == 0 || n > CAPTURE_MAX) {
			printf("ERR 2 n must be 1 to %u\n", CAPTURE_MAX);
			return;
		}
		if (trig_hz == 0) {
			printf("ERR 5 trigger is off\n");
			return;
		}
		printf("OK\n");
		Capture_Start(n); // dump is sent from main when it finishes
	}
	else if (strcmp(cmd, "STAT?") == 0) {
		if (cap_state == CAP_CAPTURING) state = "CAPTURING";
		else if (cap_state == CAP_DONE) state = "DUMPING";
		else state = "IDLE";

		// OK STAT,<trigger>,<hz>,<psc>,<arr>,<state>,<overcaptures>
		printf("OK STAT,%s,%u,%u,%u,%s,%u\n", trig_hz ? "HW" : "OFF",
			trig_hz, trig_psc, trig_arr, state, n_overcapture);
	}
	else if (strcmp(cmd, "ZERO") == 0 || strcmp(cmd, "TRIGSW") == 0 ||
	         strcmp(cmd, "PATH") == 0 || strcmp(cmd, "LOAD") == 0 ||
	         strcmp(cmd, "LATENCY") == 0) {
		printf("ERR 4 not implemented yet\n");
	}
	else {
		printf("ERR 1 unknown command\n");
	}
}
