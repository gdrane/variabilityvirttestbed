// @author Gauresh Rane (gdrane@cs.ucla.edu)
static uint64_t total_data_proc_cycle_count = 0;
static uint64_t total_branch_cycle_count = 0;
static uint64_t total_multiply_cycle_count = 0;
static uint64_t total_ldst_cycle_count = 0;
static uint64_t total_misc_cycle_count = 0;

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
	// TODO(gdrane): Take lock before accessing
	total_data_proc_cycle_count += tb->data_proc_cycle_count;
	total_branch_cycle_count += tb->branch_cycle_count;
	total_multiply_cycle_count += tb->multiply_cycle_count;
	total_ldst_cycle_count += tb->ldst_cycle_count;
	total_misc_cycle_count += tb->misc_cycle_count;
	// TODO(gdrane): Remove access lock taken
}

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