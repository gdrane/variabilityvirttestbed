#include "qemu-variability.h"
#include "qemu-timer.h"
#include "qjson.h"
#include "qint.h"

static uint64_t total_data_proc_cycle_count = 0;
static uint64_t total_branch_cycle_count = 0;
static uint64_t total_multiply_cycle_count = 0;
static uint64_t total_ldst_cycle_count = 0;
static uint64_t total_misc_cycle_count = 0;
static uint64_t total_sleep_cycle_count = 0;

// Prev energies, not the current one, current is calculated based on the 
// chpkts and cycle after that  and current frequency
static uint64_t prev_data_proc_insn_energy = 0;
static uint64_t prev_branch_insn_energy = 0;
static uint64_t prev_multiply_insn_energy = 0;
static uint64_t prev_ldst_insn_energy = 0;
static uint64_t prev_misc_insn_energy = 0;

// Cycle count Checkpoint, saved when there is a  change in frequency
static uint64_t cycle_count_chkpt_data_proc = 0;
static uint64_t cycle_count_chkpt_branch = 0;
static uint64_t cycle_count_chkpt_multiply = 0;
static uint64_t cycle_count_chkpt_ldst = 0;
static uint64_t cycle_count_chkpt_misc = 0;

// Sleep Cycle count variables
static bool started_sleep_count = false;
static int64_t sleep_start_time = 0;
static uint64_t mach_freq = 400;
static uint64_t sleep_energy = 0;
PowerModel *curr_power_model = NULL;


static void accumulate_energy(struct energy_counter *s);

void read_all_cycle_counters(struct cycle_counter *s)
{
	// TODO(gdrane): Add lock before access
	s->data_proc_cycles = total_data_proc_cycle_count;
	s->branch_cycles = total_branch_cycle_count;
	s->multiply_cycles = total_multiply_cycle_count;
	s->ldst_cycles = total_ldst_cycle_count;
	s->misc_cycles = total_misc_cycle_count;
	//TODO(gdrane): Remove access lock
}

void reset_all_cycle_counters(void) 
{
	total_data_proc_cycle_count = 0;
	total_branch_cycle_count = 0;
	total_multiply_cycle_count = 0;
	total_ldst_cycle_count = 0;
	total_misc_cycle_count = 0;
}

void increment_cycle_counters(TranslationBlock* tb)
{
	//printf("Data Processing Cycle : %llu, Branch Cycle Count: %llu, Multiply Cycle Count: %llu, LDST Cycle Count: %llu, Misc Cycle Count: %llu\n", total_data_proc_cycle_count, total_branch_cycle_count, total_multiply_cycle_count, total_ldst_cycle_count, total_misc_cycle_count);
	// TODO(gdrane): Take lock before accessing
	total_data_proc_cycle_count += tb->data_proc_cycle_count;
	total_branch_cycle_count += tb->branch_cycle_count;
	total_multiply_cycle_count += tb->multiply_cycle_count;
	total_ldst_cycle_count += tb->ldst_cycle_count;
	total_misc_cycle_count += tb->misc_cycle_count;
	// TODO(gdrane): Remove access lock taken
}

void freq_changed_set_chkpt(struct cycle_counter *s)
{
	struct energy_counter cur_energy;
	if(curr_power_model != NULL)
		curr_power_model->read_active_power(&cur_energy);
	accumulate_energy(&cur_energy);
	cycle_count_chkpt_data_proc = s->data_proc_cycles;
	cycle_count_chkpt_branch = s->branch_cycles;
	cycle_count_chkpt_multiply = s->multiply_cycles;
	cycle_count_chkpt_ldst = s->ldst_cycles;
	cycle_count_chkpt_misc = s->misc_cycles;
	if(curr_power_model != NULL)
		curr_power_model->freq_change_notify(100);
}

void read_all_cycle_counter_chkpts(struct cycle_counter *s)
{
	s->data_proc_cycles = cycle_count_chkpt_data_proc;
	s->branch_cycles = cycle_count_chkpt_branch;
	s->multiply_cycles = cycle_count_chkpt_multiply;
	s->ldst_cycles = cycle_count_chkpt_ldst;
	s->misc_cycles = cycle_count_chkpt_misc;
}

void calculate_active_energy(struct energy_counter *s)
{
	struct energy_counter prev_energy;
	if(curr_power_model != NULL)
		curr_power_model->read_active_power(s);
	read_all_prev_energy(&prev_energy);
	s->data_proc_energy += prev_energy.data_proc_energy;
	s->branch_energy += prev_energy.branch_energy;
	s->multiply_energy += prev_energy.multiply_energy;
	s->ldst_energy += prev_energy.ldst_energy;
	s->misc_energy += prev_energy.misc_energy;
}

void calculate_sleep_energy(struct energy_counter *s)
{
	if(curr_power_model != NULL)
		curr_power_model->read_sleep_power(s);
}

static void accumulate_energy(struct energy_counter *s)
{
	prev_data_proc_insn_energy += s->data_proc_energy;
	prev_branch_insn_energy += s->branch_energy;
	prev_multiply_insn_energy += s->multiply_energy;
	prev_ldst_insn_energy += s->ldst_energy;
	prev_misc_insn_energy += s->misc_energy;
}

void read_all_prev_energy(struct energy_counter *s)
{
	s->data_proc_energy = prev_data_proc_insn_energy;
	s->branch_energy = prev_branch_insn_energy;
	s->multiply_energy = prev_multiply_insn_energy;
	s->ldst_energy = prev_ldst_insn_energy;
	s->misc_energy = prev_misc_insn_energy;
}

void start_sleep_cycle_count(void) 
{
	sleep_start_time = qemu_get_clock_ns(vm_clock);
	printf("Started Sleep Cycle Count: %lld\n", sleep_start_time);
	started_sleep_count = true;
	if(curr_power_model != NULL)
		curr_power_model->sleep_start_notify();	
}

void stop_sleep_cycle_count(void)
{
	if(started_sleep_count)
	{
		printf("Stop Sleep Time passed %lld \n", (qemu_get_clock_ns(vm_clock) - sleep_start_time));
		if(curr_power_model != NULL)
			curr_power_model->sleep_stop_notify(qemu_get_clock_ns(vm_clock) - sleep_start_time);
		started_sleep_count = false;
		sleep_start_time = 0;
	}
}

void update_sleep_energy(uint64_t val)
{
	sleep_energy += val;
}

uint64_t read_sleep_energy(void)
{
	return sleep_energy;
}

void power_model_init(struct PowerModel* pwr_model)
{
	curr_power_model = pwr_model;
}

void do_info_cyclecount(Monitor *mon, QObject **ret_data)
{
	QDict *qdict;
	QObject *obj;

	qdict = qdict_new();
	obj = qobject_from_jsonf(" { 'instruction_class_idx' : 1 , "
								"'cycle_count': %" PRId64 " }" , total_data_proc_cycle_count);

	qdict_put_obj(qdict, "1", obj);
	
	obj = qobject_from_jsonf(" { 'instruction_class_idx' : 2 , "
								"'cycle_count': %" PRId64 " }" , total_branch_cycle_count);

	qdict_put_obj(qdict, "2", obj);

	obj = qobject_from_jsonf(" { 'instruction_class_idx' : 3 , "
								"'cycle_count': %" PRId64 " }" , total_multiply_cycle_count);

	qdict_put_obj(qdict, "3", obj);
	
	obj = qobject_from_jsonf(" { 'instruction_class_idx' : 4 , "
								"'cycle_count': %" PRId64 " }" , total_ldst_cycle_count);

	qdict_put_obj(qdict, "4", obj);
	
	obj = qobject_from_jsonf(" { 'instruction_class_idx' : 5 , "
								"'cycle_count': %" PRId64 " }" , total_misc_cycle_count);

	qdict_put_obj(qdict, "5", obj);
	
	*ret_data = QOBJECT(qdict);

}

void do_info_energy(Monitor *mon, QObject **ret_data)
{

}

void monitor_print_variability(Monitor *mon, const QObject *ret)
{

}
