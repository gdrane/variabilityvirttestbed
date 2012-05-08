// @author Gauresh Rane (gdrane@cs.ucla.edu)
#ifndef QEMU_VARIABILITY_H
#define QEMU_VARIABILITY_H
#include "monitor.h"
#include "qemu-common.h"
#include "power-model.h"

#define MAX_INSTRUCTIONS ((1 << 16) - 1)

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
	bool errorneous;
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
extern QDict* class_info;
extern bool error_pc_enabled, error_icount_enabled;
extern int error_pc_info[], error_icount_info[];

void init_instruction_set_map(void);
void increment_cycle_counter(void* tbptr, struct variability_instruction_set* s);
struct variability_instruction_set* get_map_entry(const char* instruction);

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

// Error stuff
bool skip_instruction(CPUState* env, TranslationBlock* tb);

// Command Line Parameters Stuff
void class_info_init(QDict* qdict);
QDict* get_class_info_ptr(void);
void update_insn_class_info(const char* class_idx, const char* insn);
void update_insn_error_info(const char* err_val, const char* insn);
void error_init_pc(int start_pc, int end_pc);
void error_init_icount(int start_icount, int end_icount);
#endif
