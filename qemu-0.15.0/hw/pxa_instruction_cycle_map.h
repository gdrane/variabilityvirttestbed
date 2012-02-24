// @author Gauresh D Rane
#ifndef PXA_INSTRUCTION_CYCLE_MAP_H
#define PXA_INSTRUCTION_CYCLE_MAP_H
#include "target-arm/arm_instruction_map.h"

void init_instruction_set_map(void);
uint64_t get_cycle_count(char* instruction);

struct variability_instruction_set* get_map_entry(const char* instruction)
{
	int i;
	for(i = 0;i < INSTRUCTIONS_AVAILABLE; ++i)
	{
		if(strcmp(insn_map[i].instruction,instruction) == 0)
			return &insn_map[i];
	}
	return NULL;
}

void init_instruction_set_map(void)
{
	
	insn_map = (struct variability_instruction_set*) arm_instructions;

	get_map_entry("ADC_reg")->cycle_count = 5;	
	get_map_entry("ADD_reg")->cycle_count = 5;	
	get_map_entry("ADD_imm")->cycle_count = 5;	
	get_map_entry("SUB_reg")->cycle_count = 5;	
	get_map_entry("SUB_imm")->cycle_count = 5;	
	get_map_entry("MOV_imm")->cycle_count = 5;	
	get_map_entry("MOV_reg")->cycle_count = 5;	
	get_map_entry("CMP_imm")->cycle_count = 5;	
	get_map_entry("LDR_imm_lit")->cycle_count = 12;	
	get_map_entry("BLX_imm")->cycle_count = 9;	
	get_map_entry("BLX_reg")->cycle_count = 9;
	get_map_entry("BX")->cycle_count = 9;
	get_map_entry("CMP_reg")->cycle_count = 5;	
	get_map_entry("NEG_reg")->cycle_count = 5;	
	get_map_entry("AND_reg")->cycle_count = 5;	
	get_map_entry("EOR_reg")->cycle_count = 5;	
	get_map_entry("LSL_reg")->cycle_count = 5;	
	get_map_entry("LSR_reg")->cycle_count = 5;	
	get_map_entry("ASR_reg")->cycle_count = 5;	
	get_map_entry("ROR_reg")->cycle_count = 5;	
	get_map_entry("TST_reg")->cycle_count = 5;	
	get_map_entry("CMN_reg")->cycle_count = 5;	
	get_map_entry("ORR_reg")->cycle_count = 5;	
	get_map_entry("MUL")->cycle_count = 7;	
	get_map_entry("MVN_reg")->cycle_count = 5;	
	get_map_entry("BIC_reg")->cycle_count = 5;	
	get_map_entry("LDR_reg")->cycle_count = 12;	
	get_map_entry("STR_reg")->cycle_count = 6;	
	get_map_entry("STRH_reg")->cycle_count = 6;	
	get_map_entry("STRB_reg")->cycle_count = 6;	
	get_map_entry("LDRH_reg")->cycle_count = 12;	
	get_map_entry("LDRB_reg")->cycle_count = 12;	
	get_map_entry("LDRSH_reg")->cycle_count = 12;	
	get_map_entry("LDRSB_reg")->cycle_count = 12;	
	get_map_entry("LDR_imm_off")->cycle_count = 12;	
	get_map_entry("STR_imm_off")->cycle_count = 6;	
	get_map_entry("LDRB_imm_off")->cycle_count = 12;	
	get_map_entry("STRB_imm_off")->cycle_count = 12;	
	get_map_entry("LDRH_imm_off")->cycle_count = 12;	
	get_map_entry("STRH_imm_off")->cycle_count = 6;	
	get_map_entry("POP")->cycle_count = 5;	
	get_map_entry("PUSH")->cycle_count = 5;	
	get_map_entry("UXTH")->cycle_count = 5;	
	get_map_entry("UXTB")->cycle_count = 5;	
	get_map_entry("SXTH")->cycle_count = 5;	
	get_map_entry("SXTB")->cycle_count = 5;	
	get_map_entry("BKPT")->cycle_count = 10;	
	get_map_entry("REV")->cycle_count = 5;	
	get_map_entry("CPS")->cycle_count = 5;	
	get_map_entry("LDRM")->cycle_count = 12;	
	get_map_entry("STM")->cycle_count = 7;	
	// get_map_entry("SVC")->cycle_count = ;	
	get_map_entry("SWI")->cycle_count = 5;
	// get_map_entry("CBZ")->cycle_count = ;	
	get_map_entry("B")->cycle_count = 5;	
	// get_map_entry("CBNZ")->cycle_count = ;	
	get_map_entry("LDRD_lit")->cycle_count = 7;	
	get_map_entry("LDRD_imm")->cycle_count = 7;	
	get_map_entry("STRD_imm")->cycle_count = 6;	
	// get_map_entry("TBH")->cycle_count = ;	
	// get_map_entry("TBB")->cycle_count = ;	
	get_map_entry("LDREX")->cycle_count = 12;	
	get_map_entry("LDREXB")->cycle_count = 12;	
	get_map_entry("LDREXH")->cycle_count = 12;	
	get_map_entry("SRS")->cycle_count = 6;	
	get_map_entry("RFE")->cycle_count = 5;	
	// get_map_entry("PHKTB")->cycle_count = ;	
	// get_map_entry("PHKBT")->cycle_count = ;	
	get_map_entry("SXTB16")->cycle_count = 5;	
	get_map_entry("UXTB16")->cycle_count = 5;	
	get_map_entry("QADD")->cycle_count = 5;	
	get_map_entry("QSUB")->cycle_count = 5;	
	// get_map_entry("RBIT")->cycle_count = ;	
	get_map_entry("REV16")->cycle_count = 5;	
	get_map_entry("REVSH")->cycle_count = 5;	
	get_map_entry("SEL")->cycle_count = 5;	
	get_map_entry("CLZ")->cycle_count = 5;	
	get_map_entry("LDREX")->cycle_count = 12;	
	get_map_entry("SMLALD")->cycle_count = 7;	
	get_map_entry("SMMUL")->cycle_count = 7;	
	get_map_entry("SMMLA")->cycle_count = 7;	
	get_map_entry("SMMLS")->cycle_count = 7;	
	get_map_entry("USAD8")->cycle_count = 7;	
	get_map_entry("SDIV")->cycle_count = 12;	
	get_map_entry("UDIV")->cycle_count = 12;	
	get_map_entry("SMLAL")->cycle_count = 7;	
	get_map_entry("UMAAL")->cycle_count = 7;	
	get_map_entry("BL")->cycle_count = 5;	
	// get_map_entry("MSR")->cycle_count = ;	
	// get_map_entry("CLREX")->cycle_count = ;	
	// get_map_entry("MRS")->cycle_count = ;	
	// get_map_entry("UBFX")->cycle_count = ;	
	get_map_entry("SSAT")->cycle_count = 7;	
	get_map_entry("SSAT16")->cycle_count = 7;	
	get_map_entry("USAT")->cycle_count = 7;	
	get_map_entry("USAT16")->cycle_count = 7;	
	get_map_entry("MOVT")->cycle_count = 5;	
	get_map_entry("MOV_imm")->cycle_count = 5;	
	get_map_entry("ADC_imm")->cycle_count = 5;
	get_map_entry("SBC_imm")->cycle_count = 5;
	get_map_entry("SBC_reg")->cycle_count = 5;
	get_map_entry("RSB_imm")->cycle_count = 5;
	get_map_entry("RSB_reg")->cycle_count = 5;
	get_map_entry("RSC_imm")->cycle_count = 5;
	get_map_entry("RSC_reg")->cycle_count = 5;
	get_map_entry("QDADD")->cycle_count = 5;
	get_map_entry("SMULL")->cycle_count = 7;
	// init_arm_instruction_set_map();
}

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
#endif
