#include <cstdint>
#include "interconnect.hpp"
#include "instruction.hpp"

// CPU class definition for PSX emulator

class CPU {

public:
	CPU(const Interconnect& inter) : interconnect(inter) { reset(inter); };
	void next_instruction();
	void set_register(uint32_t reg, uint32_t value);
	uint32_t get_register(uint32_t reg)  { return regs[reg]; };

private:
	uint32_t pc; // Program Counter
	uint32_t regs[32]; // General-purpose registers	

	Interconnect interconnect; // Interconnect for memory and I/O operations

	void reset(Interconnect inter);
	
	void decode_and_execute(uint32_t opcode);
	uint32_t read32(uint32_t address);
	void write32(uint32_t address, uint32_t value);

	void op_lui(Instruction instruct);
	void op_ori(Instruction instruct);
	void op_sw(Instruction instruct);
	void op_sll(Instruction instruct);
	void op_addiu(Instruction instruct);
};




