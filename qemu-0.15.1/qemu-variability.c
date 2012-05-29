// author Gauresh D Rane(gdrane@cs.ucla.edu)
#include "qemu-variability.h"
#include "qemu-timer.h"
#include "qjson.h"
#include "qint.h"

struct variability_instruction_set* insn_map = NULL;

uint8_t total_insn_classes = 0;

static uint64_t total_data_proc_cycle_count = 0;
static uint64_t total_branch_cycle_count = 0;
static uint64_t total_multiply_cycle_count = 0;
static uint64_t total_ldst_cycle_count = 0;
static uint64_t total_misc_cycle_count = 0;
static uint64_t total_cycle_count[MAX_INSN_CLASSES] = {0};
// static uint64_t total_sleep_cycle_count = 0;

// Prev energies, not the current one, current is calculated based on the 
// chpkts and cycle after that  and current frequency
/*
static float64 prev_data_proc_insn_energy = 0;
static float64 prev_branch_insn_energy = 0;
static float64 prev_multiply_insn_energy = 0;
static float64 prev_ldst_insn_energy = 0;
static float64 prev_misc_insn_energy = 0;
static float64 prev_leakage_energy = 0;
static float64 prev_dynamic_energy = 0;
*/
static uint64_t prev_data_proc_insn_energy = 0;
static uint64_t prev_branch_insn_energy = 0;
static uint64_t prev_multiply_insn_energy = 0;
static uint64_t prev_ldst_insn_energy = 0;
static uint64_t prev_misc_insn_energy = 0;
static uint64_t prev_insn_exec_energy[MAX_INSN_CLASSES] = {0};

// Cycle count Checkpoint, saved when there is a  change in frequency
static uint64_t cycle_count_chkpt_data_proc = 0;
static uint64_t cycle_count_chkpt_branch = 0;
static uint64_t cycle_count_chkpt_multiply = 0;
static uint64_t cycle_count_chkpt_ldst = 0;
static uint64_t cycle_count_chkpt_misc = 0;
static uint64_t cycle_count_chkpt[MAX_INSN_CLASSES] = {0};

// Sleep Cycle count variables
static bool started_sleep_count = false;
static int64_t sleep_start_time = 0;
//static uint64_t mach_freq = 400;
static uint64_t sleep_energy = 0;
PowerModel *curr_power_model = NULL;

QDict* class_info = NULL;
QList* error_info_memory= NULL;
QList* error_info_regs = NULL;
bool error_pc_enabled = false, error_icount_enabled = false;
int error_pc_info[2] = {0};
int error_icount_info[2] = {0};
bool errors_activated = false;


static void accumulate_energy(struct energy_counter *s);

uint64_t get_cycle_count(char* instruction)
{
	struct variability_instruction_set* p = get_map_entry(instruction);
	if(p == NULL)
	{
		printf("Illegal instruction sent");
		return 0;
	} else if(p->cycle_count == 0)
		printf("Instruction map not initialized");
	// TODO(gdrane): Put an exception like illegal instruction to get 
	// out of here.
	return p->cycle_count;
}

void increment_cycle_counter(void* tbptr, struct variability_instruction_set* s)
{
	TranslationBlock* tb = (TranslationBlock*) tbptr;
	//printf("Incrementing cycle counter for current tb\n");
	if(s == NULL || tb == NULL)
		return ;
	else if(s->cycle_count == 0)
		{
			// printf("Error\n");
			return;
		}
	/*
	switch(s->instruction_type)
	{
		case DATA_PROC_ISN:
							tb->data_proc_cycle_count += s->cycle_count;
							break;
		case BRANCH_ISN:
							tb->branch_cycle_count += s->cycle_count;
							break;
		case LDST_ISN: 		tb->ldst_cycle_count += s->cycle_count;
							break;
		case MULTIPLY_ISN:
							tb->multiply_cycle_count += s->cycle_count;
							break;
		case MISC_ISN:
							tb->misc_cycle_count += s->cycle_count;
							break;
	}*/
	tb->cycle_count[s->instruction_type - 1] += s->cycle_count;  
}

void increment_class_cycle_counter(uint8_t insn_class, uint8_t cycle_count)
{
	total_cycle_count[insn_class - 1] += cycle_count;
}

void read_all_cycle_counters(struct cycle_counter *s)
{
	// TODO(gdrane): Add lock before access
	int i;
	s->data_proc_cycles = total_data_proc_cycle_count;
	s->branch_cycles = total_branch_cycle_count;
	s->multiply_cycles = total_multiply_cycle_count;
	s->ldst_cycles = total_ldst_cycle_count;
	s->misc_cycles = total_misc_cycle_count;
	for(i = 0; i < total_insn_classes; ++i)
		s->cycle_count[i] = total_cycle_count[i];		
	//TODO(gdrane): Remove access lock
}

void reset_all_cycle_counters(void) 
{
	int  i;
	total_data_proc_cycle_count = 0;
	total_branch_cycle_count = 0;
	total_multiply_cycle_count = 0;
	total_ldst_cycle_count = 0;
	total_misc_cycle_count = 0;
	for(i = 0; i < total_insn_classes; ++i)
		total_cycle_count[i] = 0;
}

void increment_cycle_counters(void* opaque)
{
	TranslationBlock* tb = (TranslationBlock*) opaque;
	int i;
	//printf("Data Processing Cycle : %llu, Branch Cycle Count: %llu, Multiply Cycle Count: %llu, LDST Cycle Count: %llu, Misc Cycle Count: %llu\n", total_data_proc_cycle_count, total_branch_cycle_count, total_multiply_cycle_count, total_ldst_cycle_count, total_misc_cycle_count);
	// TODO(gdrane): Take lock before accessing
	total_data_proc_cycle_count += tb->data_proc_cycle_count;
	total_branch_cycle_count += tb->branch_cycle_count;
	total_multiply_cycle_count += tb->multiply_cycle_count;
	total_ldst_cycle_count += tb->ldst_cycle_count;
	total_misc_cycle_count += tb->misc_cycle_count;
	for(i = 0;i < total_insn_classes; ++i)
		total_cycle_count[i] += tb->cycle_count[i];
	// TODO(gdrane): Remove access lock taken
}

void freq_changed_set_chkpt(struct cycle_counter *s)
{
	int i;
	struct energy_counter cur_energy;
	if(curr_power_model != NULL)
		curr_power_model->read_active_power(&cur_energy);
	accumulate_energy(&cur_energy);
	cycle_count_chkpt_data_proc = s->data_proc_cycles;
	cycle_count_chkpt_branch = s->branch_cycles;
	cycle_count_chkpt_multiply = s->multiply_cycles;
	cycle_count_chkpt_ldst = s->ldst_cycles;
	cycle_count_chkpt_misc = s->misc_cycles;
	for(i = 0; i < total_insn_classes; ++i)
		cycle_count_chkpt[i] = s->cycle_count[i];
	if(curr_power_model != NULL)
		curr_power_model->freq_change_notify(100);
	
}

void read_all_cycle_counter_chkpts(struct cycle_counter *s)
{
	int i;
	s->data_proc_cycles = cycle_count_chkpt_data_proc;
	s->branch_cycles = cycle_count_chkpt_branch;
	s->multiply_cycles = cycle_count_chkpt_multiply;
	s->ldst_cycles = cycle_count_chkpt_ldst;
	s->misc_cycles = cycle_count_chkpt_misc;
	for(i = 0; i < total_insn_classes; ++i)
		s->cycle_count[i] = cycle_count_chkpt[i];
}

void calculate_active_energy(struct energy_counter *s)
{
	struct energy_counter prev_energy;
	int i;
	if(curr_power_model != NULL)
		curr_power_model->read_active_power(s);
	read_all_prev_energy(&prev_energy);
	s->data_proc_energy += prev_energy.data_proc_energy;
	s->branch_energy += prev_energy.branch_energy;
	s->multiply_energy += prev_energy.multiply_energy;
	s->ldst_energy += prev_energy.ldst_energy;
	s->misc_energy += prev_energy.misc_energy;
	for(i = 0;i < total_insn_classes; ++i)
		s->insn_energy[i] += prev_energy.insn_energy[i];
}

void calculate_sleep_energy(struct energy_counter *s)
{
	if(curr_power_model != NULL)
		curr_power_model->read_sleep_power(s);
}

static void accumulate_energy(struct energy_counter *s)
{
	int i;
	prev_data_proc_insn_energy += s->data_proc_energy;
	prev_branch_insn_energy += s->branch_energy;
	prev_multiply_insn_energy += s->multiply_energy;
	prev_ldst_insn_energy += s->ldst_energy;
	prev_misc_insn_energy += s->misc_energy;
	for(i = 0; i < total_insn_classes; ++i)
		prev_insn_exec_energy[i] = s->insn_energy[i];
}

void read_all_prev_energy(struct energy_counter *s)
{
	int i;
	s->data_proc_energy = prev_data_proc_insn_energy;
	s->branch_energy = prev_branch_insn_energy;
	s->multiply_energy = prev_multiply_insn_energy;
	s->ldst_energy = prev_ldst_insn_energy;
	s->misc_energy = prev_misc_insn_energy;
	for(i = 0;i < total_insn_classes; ++i)
		s->insn_energy[i] = prev_insn_exec_energy[i];
}

void start_sleep_cycle_count(void) 
{
	sleep_start_time = qemu_get_clock_ns(vm_clock);
	// printf("Started Sleep Cycle Count: %lld\n", sleep_start_time);
	started_sleep_count = true;
	if(curr_power_model != NULL)
		curr_power_model->sleep_start_notify();	
}

void stop_sleep_cycle_count(void)
{
	if(started_sleep_count)
	{
	//	printf("Stop Sleep Time passed %lld \n", (qemu_get_clock_ns(vm_clock) - sleep_start_time));
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

// TODO(gdrane) 

void power_model_init(PowerModel* pwr_model)
{
	curr_power_model = pwr_model;
	// TODO(gdrane)
	// 1. Create a new Bottom Half
	// 2. Initialize a ptimer with the bottom-half
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
	QDict *qdict;
	QObject *obj;
	struct energy_counter s;
	uint64_t dynamic_energy, leakage_energy;
	calculate_active_energy(&s);
	calculate_sleep_energy(&s);
	dynamic_energy = s.data_proc_energy + s.branch_energy +  s.multiply_energy + s.ldst_energy + s.misc_energy;
	leakage_energy = s.sleep_energy;
	qdict = qdict_new();
	obj = qobject_from_jsonf(" { 'ENERGY_TYPE' : 'DYNAMIC' , "
								"'value': %" PRId64 " }" , dynamic_energy);
	qdict_put_obj(qdict, "1", obj);
	obj = qobject_from_jsonf(" { 'ENERGY_TYPE' : 'LEAKAGE' , "
								"'value': %" PRId64 " }" , leakage_energy);
	qdict_put_obj(qdict, "2", obj);
	*ret_data = QOBJECT(qdict);	
}

void monitor_print_variability(Monitor *mon, const QObject *ret)
{

}

void class_info_init(QDict* qdict)
{
	class_info = qdict;
	total_insn_classes = qdict_size(qdict);
}

QDict* get_class_info_ptr(void)
{
	return class_info;
}

void update_insn_class_info(const char* class_idx, const char* insn)
{
	// Assuming class indexes are positive
	const QDictEntry* pentry;
	int64_t idx= -1;
	assert(class_info != NULL);
	pentry = qdict_first(class_info);
	do
	{
		QObject* temp = qdict_entry_value(pentry);		
		QDict* pdict = qobject_to_qdict(temp);
		idx = qdict_get_int(pdict, "idx");
		const char * str = qdict_get_str(pdict, "class_name");
		if(strcmp(str, class_idx) == 0)
			break;
		idx = -1;
		pentry = qdict_next(class_info, pentry);
	} while(pentry != NULL);
	assert(insn_map != NULL);
	if(idx != -1)
		get_map_entry(insn)->instruction_type = idx;
}

void update_insn_error_info(const char* boolstr, const char* insn)
{
	if(strcmp(boolstr, "y") == 0)
		get_map_entry(insn)->errorneous = 1;
	else 
		get_map_entry(insn)->errorneous = 0;
}

void error_init_pc(int start_pc, int end_pc)
{
	assert((start_pc > 0) && (end_pc > 0));
	assert(start_pc < end_pc);
	error_pc_enabled = true;
	error_pc_info[0] = start_pc;
	error_pc_info[1] = end_pc;
}

void error_init_icount(int start_icount, int end_icount)
{
	assert((start_icount > 0) && (end_icount > 0));
	assert(start_icount < end_icount);
	error_icount_enabled = true;
	error_icount_info[0] = start_icount;
	error_icount_info[1] = end_icount;
}

void error_regs_init(QList* qlist)
{
	error_info_regs = qlist;
}

void mem_error_init(QList* qlist)
{
	error_info_memory = qlist;
}

void error_activate_deactivate(bool val)
{
	errors_activated = val;
}
