// @author Gauresh Rane (gdrane@cs.ucla.edu)
#ifndef QEMU_VARIABILITY_H
#define QEMU_VARIABILITY_H
#include "qemu-common.h"

struct cycle_counter
{
	uint64_t data_proc_cycles;
	uint64_t branch_cycles;
	uint64_t multiply_cycles;
	uint64_t ldst_cycles;
	uint64_t misc_cycles;
};

struct variability_instruction_set
{
	char instruction[15];
	uint8_t cycle_count;
	uint8_t instruction_type;
};

struct variability_instruction_set* insn_map;

void reset_all_cycle_counters(void);
void increment_cycle_counters(TranslationBlock* tb);
void read_all_cycle_counters(struct cycle_counter *s); 

#endif
