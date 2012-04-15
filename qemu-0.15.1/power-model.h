#ifndef POWER_MODEL_H
#define POWER_MODEL_H
#include "qemu-common.h"

#define DEFAULT_FREQUENCY 400

struct energy_counter;

typedef struct PowerModel {
	void (*freq_change_notify) (uint64_t);	
	uint64_t (*read_active_power) (void *s);
	void (*sleep_start_notify) (void);
	void (*sleep_stop_notify) (uint64_t);
	uint64_t (*read_sleep_power) (void *s);
}PowerModel;

void power_init();

#endif
