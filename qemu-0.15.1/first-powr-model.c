#include "qemu-variability.h"
#include "power-model.h"

static PowerModel first_pwr_model = 
{
	.freq_change_notify = freq_has_changed,
	.read_active_power = serve_active_power,
	.sleep_start_notify = sleep_start_event,
	.sleep_stop_notify = sleep_stop_event,
	.read_sleep_power = serve_sleep_power
}

static uint64_t current_frequency = DEFAULT_FREQUENCY;

void power_init() 
{
	power_model_init(&first_pwr_model);	

}

void frequency_has_changed(uint64_t new_freq)
{
	current_frequency = new_freq;
}

uint64_t serve_active_power(void *opaque)
{
	struct cycle_counter cycle_count;
	struct cycle_count_chkpt;
	struct energy_counter *s = (struct energy_counter*) opaque;
	read_all_cycle_counters(&cycle_count);
	read_all_cycle_counter_chkpts(&cycle_count_chkpt);
	s->data_proc_energy = cycle_count->data_proc_cycles - cycle_count_chkpt->data_proc_cycles;
	s->branch_energy = cycle_count->branch_cycles - cycle_count_chkpt->branch_cycles;
	s->multiply_energy = cycle_count->multiply_cycles - cycle_count_chkpt->multiply_cycles;
	s->ldst_energy = cycle_count->ldst_cycles - cycle_count_chkpt->ldst_cycles;
	s->misc_energy = cycle_count->misc_cycles - cycle_count_chkpt->misc_cycles;
}

void sleep_start_event(void)
{

}

void sleep_stop_event(uint64_t interval)
{
	update_sleep_energy(interval);
}

uint64_t serve_sleep_power(uint64_t start)
{
	
}
