#include "qemu-variability.h"
#include "sysbus.h"

typedef struct {
	SysBusDevice busdev;
	qemu_irq irq;
} varmod;

static void varmod_reset(varmod *s)
{
	reset_all_cycle_counters();
}

static uint32_t varmod_read(void* opaque, target_phys_addr_t offset)
{
	struct cycle_counter cy;
	struct energy_counter active, sleep;
	calculate_active_energy(&active);
	calculate_sleep_energy(&sleep);
	read_all_cycle_counters(&cy);
	switch(offset)
	{
		case 0x00:// Lower 32 Data Proc Cycles
					return (uint32_t) (cy.data_proc_cycles);	
		case 0x04: // Upper 32 Data Proc Cycles
					return (uint32_t) (cy.data_proc_cycles >> 32);
					
		case 0x08: // Lower 32 Branch Cycles
					return (uint32_t) (cy.branch_cycles);
					
		case 0x0c:	// Upper 32 Branch Cycles
					return (uint32_t) (cy.branch_cycles >> 32);

		case 0x10: // Lower 32 Multiply Cycles
					return (uint32_t) (cy.multiply_cycles);
					
		case 0x14: // Upper 32 Multiple Cycles
					return (uint32_t) (cy.multiply_cycles >> 32);

		case 0x18: // Lower 32 Load-Store Cycles
					return (uint32_t) (cy.ldst_cycles);

		case 0x1c: // Upper 32 Load-Store Cycles
					return (uint32_t) (cy.ldst_cycles >> 32);

		case 0x20: // Lower 32 Miscellaneous Cycles
					return (uint32_t) (cy.misc_cycles);

		case 0x24: // Upper 32 Miscellaneous Cycles
					return (uint32_t) (cy.misc_cycles >> 32);

		case 0x28: // Lower 32 Data Proc Energy 
					return (uint32_t) (active.data_proc_energy);

		case 0x2c: // Upper 32 Data Proc Energy
					return (uint32_t) (active.data_proc_energy >> 32);
		
		case 0x30: // Lower 32 Branch instruction energy
					return (uint32_t) (active.branch_energy);
		
		case 0x34: // Upper 32 Branch instruction energy
					return (uint32_t) (active.branch_energy >> 32);
		
		case 0x38: // Lower 32 Multiply instruction energy
					return (uint32_t) (active.multiply_energy);
		
		case 0x3c: // Upper 32 Multiply instruction energy
					return (uint32_t) (active.multiply_energy >> 32);

		case 0x40: // Lower 32 LDST instruction energy
					return (uint32_t) (active.ldst_energy);

		case 0x44: // Upper 32 LDST instruction energy
					return (uint32_t) (active.ldst_energy >> 32);

		case 0x50: // Lower 32 MISC instruction energy
					return (uint32_t) (active.misc_energy);
		
		case 0x54: // Upper 32 MISC instruction energy
					return (uint32_t) (active.misc_energy >> 32);

		case 0x58: 
		case 0x5c:
		case 0x60:
		case 0x64:
				    break;
		case 0x68: // Lower 32 Sleep Energy
					return (uint32_t) (sleep.sleep_energy);
		case 0x6c: // Upper 32 Sleep Energy
					return (uint32_t) (sleep.sleep_energy >> 32);
		case 0x70:
					return (uint32_t) errors_activated;
	}

	return 0;
}

static void varmod_write(void *opaque, target_phys_addr_t offset, uint32_t value)
{
	switch(offset)
	{
		case 0x70: 
					errors_activated = (value & 1);
					break;
	}
}

static CPUReadMemoryFunc* const varmod_readfn[] = {
	varmod_read,
	varmod_read,
	varmod_read,
};

static CPUWriteMemoryFunc* const varmod_writefn[] = {
	varmod_write,
	varmod_write,
	varmod_write,
};

static int varmod_init(SysBusDevice *dev)
{
	int iomemtype;
	varmod* s = (varmod*) qemu_mallocz(sizeof(varmod));
	sysbus_init_irq(dev, &s->irq);
	varmod_reset(s);
	iomemtype = cpu_register_io_memory(varmod_readfn, varmod_writefn, s,
									DEVICE_NATIVE_ENDIAN);
	sysbus_init_mmio(dev, 0x1000, iomemtype);
	power_init();
	return 0;
}

static SysBusDeviceInfo varmod_info = {
	.init = varmod_init,
	.qdev.name = "varmod",
	.qdev.size = sizeof(varmod),
};

static void varmod_register_devices(void)
{
	sysbus_register_withprop(&varmod_info);
}

device_init(varmod_register_devices);
