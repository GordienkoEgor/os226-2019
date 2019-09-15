#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/ucontext.h>

#include "init.h"

int base_pointer;
int calls_amount = 0;
const int initial_value = 100000;
const int index_offset = 1000;

void os_sighnd(int sig, siginfo_t *info, void *ctx) {
	ucontext_t *uc = (ucontext_t *) ctx;

	uint16_t command = *(uint16_t *) uc->uc_mcontext.gregs[REG_RIP];

	if ((command & 0xff) == 0x8b) {
		int next_command_size;
		int index;
		int data_reg;

		if (command == 0x138b || command == 0x0b8b) {	
			data_reg = ((command & 0xff00) == 0x1300 ? REG_RDX : REG_RCX);
			index = 0;
			next_command_size = 2;
		}
		else if (command == 0x538b) {
			data_reg = REG_RDX;
			index = (*(uint32_t *) uc->uc_mcontext.gregs[REG_RIP] & 0xff0000) >> 18;
			next_command_size = 3;
		}
		else if (command == 0x558b || command == 0x4d8b) {
			data_reg = ((command & 0xff00) == 0x5500 ? REG_RDX : REG_RCX);
			index = ((*(uint32_t *) uc->uc_mcontext.gregs[REG_RIP] & 0xff0000) >> 18) \
					+ (((int)uc->uc_mcontext.gregs[REG_RBP] - base_pointer) >> 2);
			next_command_size = 3;
		}

		uc->uc_mcontext.gregs[data_reg] = initial_value + index_offset * index + (++calls_amount);
		uc->uc_mcontext.gregs[REG_RIP] += next_command_size;
	}
}

void init(void *base) {
	base_pointer = base;

	struct sigaction act = {
		.sa_sigaction = os_sighnd,
		.sa_flags = SA_RESTART,
	};
	sigemptyset(&act.sa_mask);

	sigaction(SIGSEGV, &act, NULL);
}
