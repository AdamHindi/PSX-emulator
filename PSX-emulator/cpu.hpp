#pragma once

#include <cstdint>
#include "interconnect.hpp"
#include "instruction.hpp"
#include "operations_extra.hpp"

// CPU class definition for PSX emulator
enum Exception {
	/// System c a l l ( c au sed by the SYSCALL opcode )
	SysCall = 0x8,
	Overflow = 0xc,
	LoadAddressError = 0x4,
	StoreAddressError = 0x5,
	Break = 0x9,
	CoprocessorError = 0xb,
};

class CPU {

public:
	CPU(Interconnect& inter) : interconnect(&inter), load({  }) { running = true; instruction_num=0;reset(inter); };
	void run_next_instruction();
	void set_register(uint32_t reg, uint32_t value);
	uint32_t get_register(uint32_t reg)  { return regs[reg]; };
	bool running ;
	int instruction_num;
private:
	Instruction lastInstruct = Instruction(0x0000000);
	Instruction beforeLast = Instruction(0x0000000);
	uint32_t pc; // Program Counter
	uint32_t next_pc; // Next PC for exceptions ?
	uint32_t regs[32]; // General-purpose registers	
	uint32_t out_regs[32]; // Copy for outputs for the case of load delays
	uint32_t hi;
	uint32_t lo;
	
	bool in_branch;
	bool delay_slot;

	uint32_t current_pc; // for exceptions
	uint32_t epc; // cop0 register 14
	uint32_t cause; //cop0 register 13 : cause reg
	uint32_t sr; // Cop0 register 12 : Status Register

	Interconnect* interconnect; // Interconnect for memory and I/O operations
	uint32_t next_instruction; //pipeline of next instruction to fetch before jumps

	void reset(Interconnect& inter);
	void exception(Exception cause);

	std::pair<uint32_t, uint32_t> load;
	void decode_and_execute(uint32_t opcode);
	uint32_t read32(uint32_t address);
	uint16_t read16(uint32_t address) { return interconnect->read16(address); };
	uint8_t read8(uint32_t address);
	void write32(uint32_t address, uint32_t value);

	void write16(uint32_t address, uint16_t value) {
		interconnect->write16(address, value);
	};
	void write8(uint32_t address, uint8_t value);
	void branch(int32_t off);

	void op_syscall(Instruction instruct);

	
	void op_cop0(Instruction instruct);
	void op_cop1(Instruction instruct);
	void op_cop2(Instruction instruct);
	void op_cop3(Instruction instruct);

	void op_lui(Instruction instruct);
	void op_ori(Instruction instruct);
	void op_sw(Instruction instruct);
	void op_sll(Instruction instruct);
	void op_addiu(Instruction instruct);
	void op_jmp(Instruction instruct);
	void op_or(Instruction instruct);
	void op_mtc0(Instruction instruct);
	void op_bne(Instruction instruct);
	void op_addi(Instruction instruct);
	void op_lw(Instruction instruct);
	void op_sltu(Instruction instruct);
	void op_addu(Instruction instruct);
	void op_sh(Instruction instruct);
	void op_jal(Instruction instruct);
	void op_andi(Instruction instruct);
	void op_sb(Instruction instruct);
	void op_jr(Instruction instruct);
	void op_lb(Instruction instruct);
	void op_beq(Instruction instruct);
	void op_mfc0(Instruction instruct);
	void op_and(Instruction instruct);
	void op_add(Instruction instruct);
	void op_bgtz(Instruction instruct);
	void op_blez(Instruction instruct);
	void op_lbu(Instruction instruct);
	void op_jalr(Instruction instruct);
	void op_bxx(Instruction instruct);
	void op_slti(Instruction instruct);
	void op_subu(Instruction instruct);
	void op_sra(Instruction instruct);
	void op_div(Instruction instruct);
	void op_mflo(Instruction instruct);
	void op_srl(Instruction instruct);
	void op_sltiu(Instruction instruct);
	void op_divu(Instruction instruct);
	void op_mfhi(Instruction instruct);
	void op_slt(Instruction instruct);
	void op_mtlo(Instruction instruct);
	void op_mthi(Instruction instruct);
	void op_rfe(Instruction instruct);
	void op_lhu(Instruction instruct);
	void op_sllv(Instruction instruct);
	void op_lh(Instruction instruct);
	void op_nor(Instruction instruct);
	void op_srav(Instruction instruct);
	void op_srlv(Instruction instruct);
	void op_multu(Instruction instruct);
	void op_xor(Instruction instruct);
	void op_mult(Instruction instruct);
	void op_sub(Instruction instruct);
	void op_xori(Instruction instruct);
	void op_lwl(Instruction instruct);
	void op_lwr(Instruction instruct);
	void op_swl(Instruction instruct);
	void op_swr(Instruction instruct);
	void op_break(Instruction instruct);
};	




