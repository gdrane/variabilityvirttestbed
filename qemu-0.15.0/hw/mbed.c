#include "sysbus.h"
#include "arm-misc.h"
#include "boards.h"
#include "qemu-common.h"
#include "qemu-timer.h"
#include "hw/hw.h"
#include "hw/irq.h"

//-----defines-----
#define PERIPHERAL_AREA_SIZE 0x3fff //16KB


/* Main Oscillator Of the MBED */
// Main oscillator is like the vm_clock in case of qemu because it is the one that
// provides all the timings to qemu

QEMUClock* main_oscillator;
bool main_oscillator_enabled;

static void enable_main_oscillator() 
{
	// Does nothing except gets a reference to the vm_clock maintained by
	// qemu
	if(!main_oscillator_enabled) 
	{
		main_oscillator = vm_clock;
		main_oscillator_enabled = true;
	}
}

static void disable_main_oscillator() 
{
	main_oscillator = NULL;
	main_oscillator_enabled = false;
}

static void change_oscillator_operating_range(int range) 
{
	printf("Changes in oscillator operating range not yet implemented\n");
}

/* PLL0 */


/* System Control */

typedef struct {
	/* Phase Locked Loop State */
} phase_locked_loop_state;

typedef struct {
} clock_power_state;

typedef struct {
	uint32_t flashcfg; /* Flash Accelerator State */
	/* Clock And Power Control State*/
	uint32_t pll0con; /* PLL Control Register */
	uint32_t pll0cfg; /* PLL Configuration Register */
	uint32_t pll0stat; /* PLL Status Register */
	uint32_t pll0feed; /* PLL Feed Register */
	uint32_t pll1con; /* PLL Control Register */
	uint32_t pll1cfg; /* PLL Configuration Register */
	uint32_t pll1stat; /* PLL Status Register */
	uint32_t pll1feed; /* PLL Feed Register */
	/* Power Control */
 	uint32_t pcon; /* Power Control Register */
	uint32_t pconp; /* Power Control for Peripherals Register */
	/* Clock Dividers */
	uint32_t cclkcfg; /* CPU Clock Configuration Register */
	uint32_t usbclkcfg; /* USB Clock Configurator Register */
	uint32_t clksrcsel; /* Clock Source Select Register */
	uint32_t extint; /* External Interrupt Flag Register */
	uint32_t extmode; /* External Interrupt Mode Register */
	uint32_t extpolar; /* External Interrupt Polarity Register */
	uint32_t rsid; /* Reset Source Indentification Register */
	uint32_t scs; /* System Control and Status */
	uint32_t pclksel0; /* Peripheral Clock Selection Register 0 */
	uint32_t pclksel1; /* Peripheral Clock Selection Register 1*/
	/* Utility */
	uint32_t clkoutcfg; /* Clock Output Configuration Register */
	qemu_irq irq;
} ssys_state;

static void ssys_update(ssys_state *s)
{
	qemu_set_irq(s->irq, (s->extint != 0));
}

static uint32_t ssys_read(void *opaque, target_phys_addr_t offset)
{
	ssys_state *s = (ssys_state *) opaque;
	printf("\nTrying to read\n");
	switch (offset) {
		case 0x000: /* flashcfg */
			return s->flashcfg;
		case 0x080: /* PLL0 Control Register */
			return s->pll0con;
		case 0x084: /* PLL0 Config Register */
			return s->pll0cfg;
		case 0x088: /* PLL0 Status Register */
			return s->pll0stat;
		case 0x08C: /* PLL0 Feed Register */
			return s->pll0feed;
		case 0x0A0: /* PLL1 Control Register */
			return s->pll1con;
		case 0x0A4: /* PLL1 Config Register */
			return s->pll1cfg;
		case 0x0A8: /* PLL1 Status Register */
			return s->pll1stat;
		case 0x0AC: /* PLL1 Feed Register */
			return s->pll1feed;
		case 0x0C0: /* Power Control Register */
			return s->pcon;
		case 0x0C4: /* Power Control For Peripherals Register */
			return s->pconp;
		case 0x104: /* CPU Clock Configuration Register */
			return s->cclkcfg;
		case 0x108: /* USB Clock Configurator Register */
			return s->usbclkcfg;
		case 0x10C: /* Clock Source Select Register */
			return s->clksrcsel;
		case 0x140: /* External Interrupt Flag Register */
			return s->extint;
		case 0x148: /* External Interrupt Mode Register */
			return s->extmode;
		case 0x14C: /* External Interrupt Polarity Register */
			return s->extpolar;
		case 0x180: /* Reset Source Indentification Register */
			return s->rsid;
		case 0x1A0: /* System Control and Status */
			return s->scs;
		case 0x1A8: /* Peripheral Clock Selection Register 0 */
			return s->pclksel0;
		case 0x1AC: /* Peripheral Clock Selection Register 1 */
			return s->pclksel1;
		case 0x1C8: /* Clock Output Configuration Register */
			return s->clkoutcfg;
		default:
			hw_error("ssys_read : Bad Offset 0x%x\n", (int)offset);
			return 0;
	}
}

static void ssys_write(void *opaque, target_phys_addr_t offset, uint32_t value)
{
	ssys_state *s = (ssys_state *) opaque;
	printf("Trying to write to memory\n");
	switch (offset) {
		case 0x000: /* flashcfg */
			s->flashcfg = value;
			break;
		case 0x080: /* PLL0 Control Register */
			if(value & 0x1)
			{
				// Enabling and activating PLL0
				s->pll0con |= 0x1;
				// Changing the status of the pll0 in pll0stat
				s->pll0stat |= (uint32_t)(1 << 24);
				// Assume that you get a instant lock in absence of actual
				// hardware
				s->pll0stat |= (uint32_t)(1 << 26);
			}
			else if((s->pll0con & 0x1) /*TODO(gdrane) Enter the pll0 locked condition */ && (value & 0x10))
			{
				// Updating the PLL0 Connect bit  
				s->pll0con |= 0x10;
				// Updating status in pll0stat
				s->pll0stat |= (uint32_t)(1 << 25);
				//TODO(gdrane) Code to start PLL0
			}
			break;
		case 0x084: /* PLL0 Config Register */
			{
				// Multiplier select 
				uint32_t msel0 = value &	((uint32_t)(1 << 15) - 1);
				// Divider select
				uint32_t nsel0 = value >> 16;
				if (msel0) 
				{
					s->pll0cfg &= ~ ((uint32_t)(1 << 15) - 1);
					s->pll0stat &= ~ ((uint32_t)(1 << 15) - 1);
					s->pll0cfg |= msel0 - 1;
					s->pll0stat |= msel0 - 1;
				}
				if(nsel0) 
				{
					int prevpll0stat = s->pll0stat;
					prevpll0stat &= ~ ((uint32_t)(1 << 24) - 1)
					s->pll0cfg &= ((uint32_t)(1 << 15) - 1);
					s->pll0stat &= ((uint32_t)(1 << 15) - 1); 
					nsel0 --;
					nsel0 <<= 16;
					s->pll0cfg |= nsel0;
					s->pll0stat |=nsel0;
					s->pll0stat |= prevpll0stat;
				}
				if (msel0 && value >> 16 /* I destroy nsel thus this*/)
				{
					// Wait for plock0 to become active
					// In case of software emulation we need not wait for
					// pll0 to get the lock
				}
			}
			break;
		case 0x088: /* PLL0 Status Register */
			printf("\nYou cannot write the PLL0 status register\n");
			break;
		case 0x08C: /* PLL0 Feed Register */
			if(s->pll0feed == 0 && value == 0xAA)
			{
				s->pll0feed = 0xAA;
			}
			else if(s->pll0feed == 0xAA && value == 0x55)
				 {
					s->pll0feed = value;
					// Feed Sequence completed
				 }
			break;
		case 0x0A0: /* PLL1 Control Register */
			s->pll1con = value;
			break;
		case 0x0A4: /* PLL1 Config Register */
			s->pll1cfg = value;
			break;
		case 0x0A8: /* PLL1 Status Register */
			s->pll1stat = value;
			break;
		case 0x0AC: /* PLL1 Feed Register */
			s->pll1feed = value;
			break;
		case 0x0C0: /* Power Control Register */
			s->pcon = value;
			break;
		case 0x0C4: /* Power Control For Peripherals Register */
			s->pconp = value;
			break;
		case 0x104: /* CPU Clock Configuration Register */
			s->cclkcfg = value;
			break;
		case 0x108: /* USB Clock Configurator Register */
			s->usbclkcfg = value;
			break;
		case 0x10C: /* Clock Source Select Register */
			int pll0clock = 0;
			swtich(value & 0x11) {
				case 0: 
						printf("\nInternal RC Oscillator cannot be selected as Pll0 clock: NOT IMPLEMENTED\n");
					 	break;
				case 1:
						printf("\nMain Oscillator Selected as clock\n");
						break;
				case 2:
						printf("\nRTC Oscillator cannot be used as clock source:NOT IMPLEMENTED\n");
						break;
			}
			s->clksrcsel = (uint32_t)0x01;
			break;
		case 0x140: /* External Interrupt Flag Register */
			s->extint = value;
			break;
		case 0x148: /* External Interrupt Mode Register */
			s->extmode = value;
			break;
		case 0x14C: /* External Interrupt Polarity Register */
			s->extpolar = value;
			break;
		case 0x180: /* Reset Source Indentification Register */
			s->rsid = value;
			break;
		case 0x1A0: /* System Control and Status */
			{
				int oscrange = ((s->scs >> 4) & 0x1);
				if((s->scs >> 6) & 0x1) {
					s->scs = value;
					s->scs |= 0x1000000;
				} else {
					s->scs = value;
					s->scs &= ~((uint32_t)(1 << 6));
				}
				if((s->scs >> 5) & 0x1) {
					enable_main_oscillator();
					// Setting the main oscillator started status
					s->scs |= 0x1000000;
				} else {
					disable_main_oscillator();
				}
				if(oscrange  != ((s->scs >> 4) & 0x1))
					change_oscillator_operating_range(((s->scs >> 4) & 0x1));
			}
			break;
		case 0x1A8: /* Peripheral Clock Selection Register 0 */
			s->pclksel0 = value;
			break;
		case 0x1AC: /* Peripheral Clock Selection Register 1 */
			s->pclksel1 = value;
			break;
		case 0x1C8: /* Clock Output Configuration Register */
			s->clkoutcfg = value;
			break;
		default:
			hw_error("ssys_write : Bad Offset 0x%x\n", (int)offset);
	}
	// ssys_update(s);
}

static CPUWriteMemoryFunc * const ssys_writefn[] = {
	ssys_write,
	ssys_write,
	ssys_write,
};

static CPUReadMemoryFunc * const ssys_readfn[] = {
	ssys_read,
	ssys_read,
	ssys_read,
};

static void ssys_reset(void *opaque)
{
	ssys_state *s = (ssys_state *)opaque;

	s->extint = 0;
	s->extmode = 0;
	s->extpolar = 0;
	// TODO(gdrane):  Figure out where to implement s->rsid complex logic
	s->scs = 0;
	s->clksrcsel = 0;
	s->pll0con = 0;
	s->pll0cfg = 0;
	s->pll0stat = 0;
	s->pll0feed = 0;
	s->pll1con = 0;
	s->pll1cfg = 0;
	s->pll1stat = 0;
	s->pll1feed = 0;
	s->cclkcfg = 0;
	s->usbclkcfg = 0;
	s->pclksel0 = 0;
	s->pclksel1 = 0;
	s->pcon = 0;
	s->flashcfg = 0x303A;
	s->pconp = 0x03BE;
	disable_main_oscillator();
}

static void ssys_calculate_system_clock(ssys_state *s)
{
	// TODO(gdrane) : Stellaris implements this check out if mbed requires
	// this
}

static int mbed_sys_post_load(void *opaque, int version_id)
{
	ssys_state *s = opaque;

	ssys_calculate_system_clock(s);

	return 0;
}

static const VMStateDescription vmstate_mbed_sys = {
	.name = "mbed_sys",
	.version_id = 1,
	.minimum_version_id = 1,
	.post_load = mbed_sys_post_load,
	.fields = (VMStateField[]) {
		VMSTATE_UINT32(flashcfg, ssys_state), 
 		VMSTATE_UINT32(pll0con, ssys_state),
 		VMSTATE_UINT32(pll0cfg, ssys_state),
 		VMSTATE_UINT32(pll0stat, ssys_state),
 		VMSTATE_UINT32(pll0feed, ssys_state),
 		VMSTATE_UINT32(pll1con, ssys_state),
 		VMSTATE_UINT32(pll1cfg, ssys_state),
		VMSTATE_UINT32(pll1stat, ssys_state),
		VMSTATE_UINT32(pll1feed, ssys_state),
		VMSTATE_UINT32(cclkcfg, ssys_state),
		VMSTATE_UINT32(usbclkcfg, ssys_state),
		VMSTATE_UINT32(clksrcsel, ssys_state),
		VMSTATE_UINT32(extint, ssys_state),
		VMSTATE_UINT32(extmode, ssys_state),
		VMSTATE_UINT32(extpolar, ssys_state),
		VMSTATE_UINT32(rsid, ssys_state),
		VMSTATE_UINT32(scs, ssys_state),
		VMSTATE_UINT32(pclksel0, ssys_state),
		VMSTATE_UINT32(pclksel1, ssys_state),
		VMSTATE_UINT32(clkoutcfg, ssys_state),
		VMSTATE_END_OF_LIST()
	}
};

static int mbed_sys_init(uint32_t base, qemu_irq irq)
{
	int iomemtype;
	ssys_state *s;
	s = (ssys_state *)qemu_mallocz(sizeof(ssys_state));
	s->irq = irq;
	iomemtype = cpu_register_io_memory(ssys_readfn, 
									   ssys_writefn, (void*)s,
									   DEVICE_NATIVE_ENDIAN);
	printf("\niomemtype: %d", iomemtype);
	cpu_register_physical_memory(base, PERIPHERAL_AREA_SIZE, iomemtype);
	ssys_reset(s);
	vmstate_register(NULL, -1, &vmstate_mbed_sys, s);
	return 0;
}

// Machine initialization routine
static void mbed_init(ram_addr_t ram_size,
				const char *boot_device,
				const char *kernel_filename, const char *kernel_cmdline,
				const char *initrd_filename, const char *cpu_model)
{
	// CPUState *env;
	// ram_addr_t ram_offset;
	qemu_irq *cpu_pic;
	// TODO(gdrane): Figure out gpio and pic on mbed
	// qemu_irq pic[32];
	// qemu_irq sic[32];
	// qemu_irq adc;
	// DeviceState *dev;
	// flash size = 512kb = 0x200	
	int flash_size = 0x200;
	// sram_size = 32kb  = 0x20
	int sram_size = 0x20;
	
	cpu_pic = armv7m_init(flash_size, sram_size, kernel_filename, cpu_model);
	// Adding a ADC
	// dev = sysbus_create_varargs("mbed-adc", 0x40034000, 
	//							/* TODO(gdrane): find pic that should be connected*/);
	// qdev_connect_gpio_in(dev, 0);

	// Adding DAC
	// dev = sysbus_create_varargs("mbed-dac", 0x4008C000,
	//							/* TODO(gdrane): Find the dac's to connect to*/);
	// qdev_connect_gpio_in(dev, 0);
	#define SYS_CNTRL_INTRPT_NO 28	
	mbed_sys_init(0x400fc000, cpu_pic[SYS_CNTRL_INTRPT_NO]);
}

static QEMUMachine mbed_machine = {
	.name = "mbed",
	.desc = "MBED LPC 1768",
	.init = mbed_init,
};

static void mbed_machine_init(void)
{
	qemu_register_machine(&mbed_machine);
}

machine_init(mbed_machine_init);

// TODO(gdrane): Define and Register devices
