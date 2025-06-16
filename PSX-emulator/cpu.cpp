

#include "cpu.hpp"



void CPU::reset(Interconnect inter) {
	pc = 0xbfc00000; // Reset program counter to the start of BIOS:  0xbfc00000
	interconnect = inter; // Initialize the interconnect (contains BIOS for now)
	for (int i = 1; i < 32; ++i) { // Set all registers to 0 except for $zero which is always 0
		regs[i] = 0;
	}
	regs[0] = 0; // $zero register is always 0
}

void CPU::next_instruction() {
	uint32_t instruction = read32(pc);
	pc += 4; // Increment program counter by 4 bytes for the next instruction
	decode_and_execute(instruction);
}

void CPU::decode_and_execute(uint32_t instruction) {
	// Decode the instruction and execute it
	Instruction instruct = Instruction(instruction); // Assuming Instruction is a class that can interpret the instruction
	switch (instruct.funct) { // Example: using the last byte as an opcode
		case 0b001111: // Example opcode for LUI (Load Upper Immediate)
			op_lui(instruct);
			break;
		case 0b001101: // Example opcode for ORI (OR Immediate)
			op_ori(instruct);
			break;
		case 0b101011 : // Example opcode for SW (Store Word)
			op_sw(instruct);
			break;
		default:
			printf("Unknown instruction: 0x%08X\n", instruction);
			// Handle unknown instruction
			break;
	}
}

uint32_t CPU::read32(uint32_t address) {
	return CPU::interconnect.read32(address); // Read a 32-bit value from the interconnect
}

void CPU::set_register(uint32_t reg, uint32_t value) {
	regs[reg] = value; // Set the register to the given value
	regs[0] = 0; // Ensure $zero register is always 0
}



void CPU::op_lui(Instruction instruct) {
	auto i = instruct.immediate; // Extract immediate value from the instruction
	auto rt = instruct.rt; // Extract the target register from the instruction
	auto v = i << 16; // Load the immediate value into the upper half of the target register
	set_register(rt, v); // Set the target register with the value
}
void CPU::op_ori(Instruction instruct) {
	auto i = instruct.immediate; // Extract immediate value from the instruction
	auto rs = instruct.rs; // Extract the source register from the instruction
	auto rt = instruct.rt; // Extract the target register from the instruction
	auto v = get_register(rs) | i; // Perform OR operation with the immediate value
	set_register(rt, v); // Set the target register with the result
}
void CPU::op_sw(Instruction instruct) {
	auto rt = instruct.rt; // Extract the target register from the instruction
	auto rs = instruct.rs; // Extract the base register from the instruction
	auto offset = instruct.immediate; // Extract the offset from the instruction
	auto address = CPU::get_register(rs) + offset; // Calculate the effective address
	auto value = CPU::get_register(rt); // Get the value to store from the target register
	interconnect.write32(address, value); // Write the value to memory at the calculated address
}