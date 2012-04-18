// @author Gauresh Rane (gdrane@cs.ucla.edu)
#ifndef QEMU_VARIABILITY_H
#define QEMU_VARIABILITY_H
#include "monitor.h"
#include "qemu-common.h"
#include "power-model.h"

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

struct energy_counter
{
	uint64_t data_proc_energy;
	uint64_t branch_energy;
	uint64_t multiply_energy;
	uint64_t ldst_energy;
	uint64_t misc_energy;
	uint64_t sleep_energy;
};

struct variability_instruction_set* insn_map;

void reset_all_cycle_counters(void);
void increment_cycle_counters(void* tb);
void read_all_cycle_counters(struct cycle_counter *s); 
void freq_changed_set_chkpt(struct cycle_counter *s);
void read_all_cycle_counter_chkpts(struct cycle_counter *s);
void read_all_prev_energy(struct energy_counter *s);
void start_sleep_cycle_count(void);
// void stop_sleep_cycle_count(void);
void do_info_cyclecount(Monitor *mon, QObject **ret_data);
void do_info_energy(Monitor *mon, QObject **ret_data);
void monitor_print_variability(Monitor *mon, const QObject *data);
void update_sleep_energy(uint64_t val);
uint64_t read_sleep_energy(void);
void power_model_init(PowerModel *pwr_model);
void calculate_sleep_energy(struct energy_counter *s);
void calculate_active_energy(struct energy_counter *s);

#endif
