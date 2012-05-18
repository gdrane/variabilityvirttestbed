#include "qemu-variability.h"

static uint8_t count_exec = 0;

int error_model(CPUState * env, TranslationBlock* tb)
{
	int i;
	count_exec ++;

	/*
		if(count_exec == 10)
		{
			env->regs[15] += 2;
			return true;
		}
	*/
	printf("\nINSN : %s", insn_map[tb->insn_under_exec].instruction);
	for(i = 0; i < 8; i += 2) {
		if(tb->args[i] == 0)
			printf(" Reg Used : %d", tb->args[i+1]);
		else if(tb->args[i] == 1)
				printf(" Immediate Offset: 0x%08x", tb->args[i+1]);
	}
	return -1;
}
