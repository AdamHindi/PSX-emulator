#include <cstdint>


// This file defines the Instruction structure used in the PS1 simulator.
struct Instruction {
	uint32_t opcode;  // The operation code of the instruction
	uint32_t rs;      // Source register
	uint32_t rt;      // Target register
	uint32_t rd;      // Destination register
	uint32_t shamt;   // Shift amount
	uint32_t funct;   // Function code for R-type instructions
	uint32_t immediate; // Immediate value for I-type instructions
	uint32_t address;  // Address for J-type instructions
	//little-endian
	Instruction(uint32_t code) {
		funct = code >> 26;
		rt = (code >> 16) & 0x1F;
		rs = (code >> 21) & 0x1F;
		immediate = code & 0xFFFF;

	};

};