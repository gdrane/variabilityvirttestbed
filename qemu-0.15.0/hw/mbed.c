#include "sysbus.h"
#include "arm-misc.h"
#include "boards.h"
#include "qemu-common.h"
#include "qemu-timer.h"
#include "hw/hw.h"
#include "hw/irq.h"

//-----defines-----
#define PERIPHERAL_AREA_SIZE 0x3fff //16KB

/* TIMER MODULE */
typedef struct timer_state {
	SysBusDevice busdev;
	uint32_t intr_reg;
	uint32_t intr_mask;
	uint32_t timer_cntrl_reg;
	uint32_t timer_counter;
	uint32_t prescale_reg;
	uint32_t prescale_counter;
	uint32_t match_cntrl_reg;
	uint32_t match_reg[3];
	uint32_t cap_cntrl_reg;
	uint32_t capture_reg[2];
	uint32_t external_match_reg;
	uint32_t count_cntrl_reg;
	int64_t tick;
	QEMUTimer *timer;
	qemu_irq irq;
}timer_state;

static void timer_update_irq(timer_state *s)
{
	int level;
	//level = (s->intr_reg & s->intr_mask) != 0;
	qemu_set_irq(s->irq, 0);
}

static void timer_reload(timer_state* s, int reset)
{
	int64_t tick;
	if(reset)
		tick = qemu_get_clock_ns(vm_clock);
	else 
		tick = s->tick;
	if ((s->count_cntrl_reg & 0x3) == 0)
	{
		uint32_t count;
		count = 1;
		tick += (int64_t)count * system_clock_scale;
		s->tick = tick;
		qemu_mod_timer(s->timer, tick);
	} else {
		hw_error("Counter Mode Not Supported\n");
	}
}

static void timer_stop(timer_state *s)
{
	qemu_del_timer(s->timer);
}

static uint32_t timer_read(void *opaque, target_phys_addr_t offset)
{
	timer_state *s = (timer_state*) opaque;
	switch (offset)
	{
		case 0x00:
					return s->intr_reg;
		case 0x04:
					return s->timer_cntrl_reg;
		case 0x08:
					printf("Timer Counter Read");
					return s->timer_counter;
		case 0x0c:
					return s->prescale_reg;
		case 0x10:
					return s->prescale_counter;
		case 0x14:
					return s->match_cntrl_reg;
		case 0x18:
					return s->match_reg[0];
		case 0x1c:
					return s->match_reg[1];
		case 0x20:
					return s->match_reg[2];
		case 0x24:
					return s->match_reg[3];
		case 0x28:
					return s->cap_cntrl_reg;
		case 0x2c:
					return s->capture_reg[0];
		case 0x30:
					return s->capture_reg[1];
		case 0x3c:
					return s->external_match_reg;
		case 0x70:
					return s->count_cntrl_reg;
		default:
				return 0;
	}
	return 0;
}

static void timer_write(void *opaque, target_phys_addr_t offset, uint32_t value)
{
	timer_state *s = (timer_state*)opaque;
	printf("\nTimer Write");
	switch(offset)
	{
		case 0x00:
				s->intr_mask |= value;
				break;
		case 0x04:
				printf("Timer Clock Enable update%d\n",value);
				s->timer_cntrl_reg = value;
				if((s->timer_cntrl_reg & 0x3) == 0x1) {
					timer_reload(s, 1);
				}
				printf("Timer Clock Enable update%d\n",s->timer_cntrl_reg);
				/*
				if(value & 0x1)
				{
					s->timer_cntrl_reg |= value & 0x1; 
				} else if(!(value & 0x1)) 
						{
							s->timer_cntrl_reg &= ~((uint32_t)1);
						}
				if(value & 0x2)
				{
					s->timer_cntrl_reg |= value & 0x2;		
				} else if(!(value & 0x2))
						{
							s->timer_cntrl_reg &= ~((uint32_t)(1<<1));
							// Starting Clock Ticks
							timer_reload(s, 1);
						}
				*/
				break;
		case 0x08:
				s->timer_counter = value;
				break;
		case 0x0c:
				s->prescale_reg = value;
				break;
		case 0x10:
				s->prescale_counter = value;
				break;
		case 0x14:
				s->match_cntrl_reg = value;
				break;
		case 0x18:
				s->match_reg[0] = value;
				break;
		case 0x1c:
				s->match_reg[1] = value;
				break;
		case 0x20:
				s->match_reg[2] = value;
				break;
		case 0x24:
				s->match_reg[3] = value;
				break;
		case 0x28:
				s->cap_cntrl_reg |= value & ((1<<6) - 1);	
				break;
		case 0x2c:
				printf("Cannot write to capture register 0\n");
				break;
		case 0x30:
				printf("Cannot write to capture register 1\n");
				break;
		case 0x3c:
				hw_error("External Match Not Emulated\n");
				// Not emulated
				return;
		case 0x70:
				//if (value && ~0x3)
				//	return;
				s->count_cntrl_reg |= (value & 0x3);	
				s->count_cntrl_reg |= (value & (0x3 << 2));
				break;
	}
	timer_update_irq(s);
}

static CPUReadMemoryFunc * const timer_readfn[] = {
	timer_read,
	timer_read,
	timer_read
};

static CPUWriteMemoryFunc * const timer_writefn[] = {
	timer_write,
	timer_write,
	timer_write
};

static void mbed_timer_tick(void *opaque)
{
	printf("One Clock Tick\n");
	timer_state *s = (timer_state*) opaque;
	// Check if timer/counter is enabled
	if (!(s->timer_cntrl_reg & 0x1)) {
		printf("Counter not enabled:0x%x\n",s->timer_cntrl_reg);
		return;
	}
	// Reset Timer Counter and Prescale Counter till TCR[1] is not zero
	if (s->timer_cntrl_reg & 0x2) {
		s->timer_counter = 0;
		s->prescale_counter = 0;
		printf("Reset Timersn\n");
		return;
	}
	printf("Incrementing prescale counter\n");
	if((s->count_cntrl_reg & 0x3) == 0) {
		// Timer Mode
		// Increment Prescale counter
		s->prescale_counter++;
		// Check whether it's value reached prescale register
		if(s->prescale_counter > s->prescale_reg) {
			// Increment Timer Counter by on
			s->timer_counter ++;
			s->prescale_counter = 0;
			if(s->timer_counter == s->match_reg[0]) {
				if(s->match_cntrl_reg & 0x2) {
					s->timer_counter = 0;
				}
				if(s->match_cntrl_reg & 0x1) {
					qemu_irq_pulse(s->irq);
				}
				if(s->match_cntrl_reg & 0x4)
				{
					timer_stop(s);
					return;
				}
				// TODO(gdrane) Stopping pc and tc and setting tcr[0] to 0 
				// call timer_stop
			} else if(s->timer_counter == s->match_reg[1]) {	
				if(s->match_cntrl_reg & (1<<4))
				{
					s->timer_counter = 0;
				}
				if(s->match_cntrl_reg & (1<<3))
				{
					qemu_irq_pulse(s->irq);
				}
				if(s->match_cntrl_reg & (1<<5))
				{
					timer_stop(s);
					return;
				}
			} else if(s->timer_counter == s->match_reg[2]) {
				if(s->match_cntrl_reg & (1<<7))
				{
					s->timer_counter = 0;
				}
				if(s->match_cntrl_reg & (1<<6))
				{
					qemu_irq_pulse(s->irq);
				}
				if(s->match_cntrl_reg & (1<<8))
				{
					timer_stop(s);
					return;
				}
			} else if(s->timer_counter == s->match_reg[3]) {
				if(s->match_cntrl_reg & (1<<10))
				{
					s->timer_counter = 0;
				}
				if(s->match_cntrl_reg & (1<<9))
				{
					qemu_irq_pulse(s->irq);
				}
				if(s->match_cntrl_reg & (1<<11))
				{
					timer_stop(s);
					return;
				}
			}
			timer_reload(s, 0);
		}
	} else {
		// Counter Mode
		// Not Implemented
	}
	timer_update_irq(s);
}

static void mbed_timer_reset(timer_state *s)
{
	s->intr_reg = 0;
	s->intr_mask = 0;
 	s->timer_cntrl_reg = 0;
	s->timer_counter = 0;
 	s->prescale_reg = 0;
 	s->prescale_counter = 0;
 	s->match_cntrl_reg = 0;
 	s->match_reg[0] = 0;
 	s->match_reg[1] = 0;
 	s->match_reg[2] = 0;
 	s->match_reg[3] = 0;
 	s->cap_cntrl_reg = 0;
 	s->capture_reg[0] = 0;
 	s->capture_reg[1] = 0;
 	s->external_match_reg = 0;
 	s->count_cntrl_reg = 0;
	s->tick = 0;
}

static int mbed_timer_init(SysBusDevice *dev)
{
	printf("MBED Timer initialized\n");
	int iomemtype;
	timer_state *s = FROM_SYSBUS(timer_state, dev);
	sysbus_init_irq(dev, &s->irq);
	mbed_timer_reset(s);
	// wrong For sending interrupts on match
	 //qdev_init_gpio_out(&dev->qdev, &s->match_trigger, 1);
	// TODO(gdrane): Add incoming interrupt for capture register
	iomemtype = cpu_register_io_memory(timer_readfn,
										timer_writefn, s,
										DEVICE_NATIVE_ENDIAN);
	sysbus_init_mmio(dev, 0x4000, iomemtype);
	s->timer = qemu_new_timer_ns(vm_clock, mbed_timer_tick, &s);
	return 0;
}

/* Main Oscillator Of the MBED */
// Main oscillator is like the vm_clock in case of qemu because it is the one that
// provides all the timings to qemu
QEMUClock* main_oscillator;
bool main_oscillator_enabled = false;

static void enable_main_oscillator(void) 
{
	printf("In enable main oscillator\n");
	// Does nothing except gets a reference to the vm_clock maintained by
	// qemu
	if(!main_oscillator_enabled) 
	{
		main_oscillator = vm_clock;
		main_oscillator_enabled = true;
	}
}

static void disable_main_oscillator(void) 
{
	printf("In disable main oscillator\n");
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
	printf("In qemu ssys_update\n");
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
			else if((s->pll0con & 0x1) /*TODO(gdrane) Enter the pll0 locked condition */ && (value & 0x2))
			{
				// Updating the PLL0 Connect bit  
				s->pll0con |= 0x2;
				// Updating status in pll0stat
				s->pll0stat |= (uint32_t)(1 << 25);
				//TODO(gdrane) Code to start PLL0
			}
			break;
		case 0x084: /* PLL0 Config Register */
			{
				if(s->pll0con & 0x1)
				{
					printf("PLL0 not enabled please enable PLL0 first");
					return;
				}
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
					prevpll0stat &= ~ ((uint32_t)(1 << 24) - 1);
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
			printf("Updating feed register\n");
			// This is of no importance to a software emulator as we can go
			// ahead without the feed sequence
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
			// Used to divide PLL0 output before plugging it to CPU
			// For cclkcfg = 0, 1 this change cannot happen when PLL0 is 
			// connected
			if (s->cclkcfg  == 0 && s->cclkcfg == 1) 
			{
				if(!(s->pll0stat & (uint32_t)(1 << 25)))
				{
					printf("PLL0 enabled cannot change configuration\n");
					break;
				}
			}
			s->cclkcfg = 0;
			s->cclkcfg |= value & (((uint32_t) 1 << 8) - 1);
			break;
		case 0x108: /* USB Clock Configurator Register */
			s->usbclkcfg = value;
			break;
		case 0x10C: /* Clock Source Select Register */
			{
				// int pll0clock = 0;
				switch(value & 0x3) {
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
			}
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
				// int oscrange = ((s->scs >> 4) & 0x1);
				uint32_t oscen = (value >> 5) & 0x1;
				uint32_t oscrange = (value >> 6) & 0x1;
				if(oscrange != ((s->scs >> 6) & 0x1))
				{
					if(oscrange) 
					{
						s->scs |= ((uint32_t)1 << 6);
					}
					else
					{
						s->scs &= ~((uint32_t)1 << 6);
					}
				}
				if(oscen)
				{
					s->scs |= (1 << 5);
				}
				if ((s->scs >> 5) & 0x1) {
					enable_main_oscillator();
					
					// Setting the main oscillator started status
					s->scs |= 0x40;
				} else {
					disable_main_oscillator();
				}
				/*if(oscrange  != ((s->scs >> 4) & 0x1))
					change_oscillator_operating_range(((s->scs >> 4) & 0x1));
				*/
			}
			break;
		case 0x1A8: /* Peripheral Clock Selection Register 0 */
			// Controls the input clock to peripherals. Clock rate given to
			// peripheral is a derivative of cpuclock CCLK as per arm user 
			// manual
			// Each peripheral has 1bits reserved
			// Btw, only peripheral that is not controlled by this configuration	
			// is RTC, it's input is fixed at 1khz and or CCLK/8 (ambiguous???
			// )
			// 2 bit representation
			// 00 = CCLK/4
			// 01 = CCLK
			// 10 = CCLK/2
			// 11 = CCLK / 8 except for CAN1, CAN2 and CAN filtering when 11 selects
			// =CCLK/6 (I don't know what they are saying)
			{
				// int prevpclksel = s->pclksel0;
				printf("PCLK register 0 written\n");
				// Not interested in simulating any of these peripherals
				// you guys are free to do whatever you want to
				s->pclksel0 = value;
			}
			break;
		case 0x1AC: /* Peripheral Clock Selection Register 1 */
			// Check out the comments for pclk register 0
			{
				// Checking whether there are timer updates to GPIO interrupts
				//
				if(value & 0xC)
				{
					//int prevpclksel1 = s->pclksel1;
					printf("GPIO clock updated\n");
					// Change clock configuration of GPIO interrupts
					// TODO(gdrane)
				}
				s->pclksel1 = value;

			}
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
	printf("In system reset\n");
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
	printf("In calculate system clock\n");
	// Derived From SystemInit in system_LPC17xx.c
	// TODO(gdrane): Calculate actual value everytime using PLL mathematics
	system_clock_scale = 18;
}

static int mbed_sys_post_load(void *opaque, int version_id)
{
	printf("In mbed post load\n");
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
	printf("in mbed sys init\n");
	int iomemtype;
	ssys_state *s;
	s = (ssys_state *)qemu_mallocz(sizeof(ssys_state));
	s->irq = irq;
	iomemtype = cpu_register_io_memory(ssys_readfn, 
									   ssys_writefn, (void*)s,
									   DEVICE_NATIVE_ENDIAN);
	printf("\niomemtype: %d", iomemtype);
	system_clock_scale = 18;
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
	static const int timer_irq[] = { 1, 2, 3, 4};
	// TODO(gdrane): Figure out gpio and pic on mbed
	// qemu_irq pic[32];
	// qemu_irq sic[32];
	// qemu_irq adc;
	DeviceState *dev;
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
	
	// MBED Timer 0
		dev = sysbus_create_simple("mbed-timer0", 0x40004000, cpu_pic[timer_irq[0]]);
	// MBED Timer 1
		sysbus_create_simple("mbed-timer1", 0x40008000, cpu_pic[timer_irq[1]]);
	// MBED Timer 2
		sysbus_create_simple("mbed-timer2", 0x40090000, cpu_pic[timer_irq[2]]);
	// MBED Timer 3
		sysbus_create_simple("mbed-timer3", 0x40094000, cpu_pic[timer_irq[4]]);
	
	#define SYS_CNTRL_INTRPT_NO 28	
	mbed_sys_init(0x400fc000, cpu_pic[SYS_CNTRL_INTRPT_NO]);
	// Initializing GPIO's
	// dev = sysbus_create_varargs("mbed"
}

static void mbed_register_devices(void) {
	sysbus_register_dev("mbed-timer0", sizeof(timer_state),
						 mbed_timer_init);
	sysbus_register_dev("mbed-timer1", sizeof(timer_state),
						mbed_timer_init);
	sysbus_register_dev("mbed-timer2", sizeof(timer_state),
						mbed_timer_init);
	sysbus_register_dev("mbed-timer3", sizeof(timer_state),
						mbed_timer_init);
}

device_init(mbed_register_devices);

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
