
#include <cstdio> // Include for printf

#include "cpu.hpp"



void CPU::reset(Interconnect& inter) {
	pc = 0xbfc00000; // Reset program counter to the start of BIOS:  0xbfc00000
	next_pc = pc + 4;
	interconnect = &inter; // Initialize the interconnect (contains BIOS for now)
	for (int i = 1; i < 32; ++i) { // Set all registers to 0 except for $zero which is always 0
		regs[i] = 0;
	}
	regs[0] = 0; // $zero register is always 0
	sr = 0;
	std::copy(std::begin(regs), std::end(regs), std::begin(out_regs)); // Copy regs to out_regs 
	load.first = 0;
	load.second = 0;
	next_instruction = read32(pc);
	in_branch = false;
	delay_slot = false;
}
void CPU::branch(int32_t imm_se) {
	// MIPS branch immediates are in instructions, so shift left 2:

	int32_t offs = imm_se << 2;
	next_pc = uint32_t(int32_t(pc) + offs) ;
	in_branch = true;
}

void CPU::run_next_instruction() {
    
	/* ---------- 1. Retire previous cycle ---------- */
	for (int i = 1; i < 32; ++i) {          // r0 stays 0
		regs[i] = out_regs[i];
		out_regs[i] = regs[i];              // prime WB shadow
	}

	if (load.first) {                       // resolve load delay
		regs[load.first] = load.second;
		out_regs[load.first] = load.second; // keep shadow coherent
		load.first = 0;
	}
	current_pc = pc;
	if (current_pc % 4 != 0) {
		exception(Exception::LoadAddressError);
		return;
	}
	delay_slot = in_branch;
	in_branch = false;

	uint32_t instruction = read32(pc);
	instruction_num += 1;

	pc = next_pc;
	next_pc += 4;
	
    

   
    decode_and_execute(instruction);
}

void CPU::decode_and_execute(uint32_t instruction) {
	//printf("Instruction : %08x :", instruction);
	// Decode the instruction and execute it
	Instruction instruct = Instruction(instruction); // Assuming Instruction is a class that can interpret the instruction
	beforeLast = lastInstruct;
	lastInstruct = instruct;
	switch (instruct.funct) { // Example: using the last byte as an opcode
		case 0b000000: {
			//printf(" SUBfunction : %08x\n", instruct.subfunct);
			switch (instruct.subfunct) {
				
				case 0b000000:
					op_sll(instruct);
					break;
				case 0b000010:
					op_srl(instruct);
					break;
				case 0b000011:
					op_sra(instruct);
					break;
				case 0b000100:
					op_sllv(instruct);
					break;
				case 0x06:
					op_srlv(instruct);
					break;
				case 0x07:
					op_srav(instruct);
					break;
				case 0x08:
					op_jr(instruct);
					break;
				case 0x25:
					op_or(instruct);
					break;
				case 0x2B:
					op_sltu(instruct);
					break;
				case 0x21:
					op_addu(instruct);
					break;
				case 0x24:
					op_and(instruct);
					break;
				case 0x20:
					op_add(instruct);
					break;
				case 0x09:
					op_jalr(instruct);
					break;
				case 0x23:
					op_subu(instruct);
					break;
				case 0x1A:
					op_div(instruct);
					break;
				case 0x12:
					op_mflo(instruct);
					break;
				case 0x1B:
					op_divu(instruct);
					break;
				case 0x10:
					op_mfhi(instruct);
					break;
				case 0x2A:
					op_slt(instruct);
					break;
				case 0x0C:
					op_syscall(instruct);
					break;
				case 0x13:
					op_mtlo(instruct);
					break;
				case 0x11:
					op_mthi(instruct);
					break;
				case 0x27:
					op_nor(instruct);
					break;
				case 0x18:
					op_mult(instruct);
					break;
				case 0x19:
					op_multu(instruct);
					break;
				case 0x26:
					op_xor(instruct);
					break;
				case 0x22:
					op_sub(instruct);
					break;
				
				default:
					printf("\nUnknown subbbinstruction: 0x%08X, full 0x%08X\n", instruct.subfunct, instruction);
					running = false;
					break;
			}

				
			break;
		}
		case 0xF: // Example opcode for LUI (Load Upper Immediate)
			op_lui(instruct);
			break;
		case 0b001101: // Example opcode for ORI (OR Immediate)
			op_ori(instruct);
			break;
		case 0x2b : // Example opcode for SW (Store Word)
			op_sw(instruct);
			break;
		case 0x09:
			op_addiu(instruct);
			break;
		case 0x02:
			op_jmp(instruct);
			break;
		case 0x10:
			op_cop0(instruct);
			break;
		case 0x05:
			op_bne(instruct);
			break;
		case 0x08:
			op_addi(instruct);
			break;
		case 0x23:
			op_lw(instruct);
			break;
		case 0x29:
			op_sh(instruct);
			break;
		case 0x03:
			op_jal(instruct);
			break;
		case 0x0C:
			op_andi(instruct);
			break;
		case 0x28:
			op_sb(instruct);
			break;
		case 0x20:
			op_lb(instruct);
			break;
		case 0x04:
			op_beq(instruct);
			break;
		case 0x07:
			op_bgtz(instruct);
			break;
		case 0x06:
			op_blez(instruct);
			break;
		case 0x24:
			op_lbu(instruct);
			break;
		case 0x01:
			op_bxx(instruct);
			break;
		case 0x0A:
			op_slti(instruct);
			break;
		case 0x0B:
			op_sltiu(instruct);
			break;
		case 0x25:
			op_lhu(instruct);
			break;
		case 0x21:
			op_lh(instruct);
			break;
		case 0x11:
			op_cop1(instruct);
			break;
		case 0x12:
			op_cop2(instruct);
			break;
		case 0x13:
			op_cop3(instruct);
			break;
		case 0x22:
			op_lwl(instruct);
			break;
		case 0b100110:
			op_lwr(instruct);
			break;
		case 0x2A:
			op_swl(instruct);
			break;
		case 0x2E:
			op_swr(instruct);
			break;
		case 0x0e:
			op_xori(instruct);
			break;
		default:
			printf("\nUnknown instruction: 0x%08X, full 0x%08X\n", instruct.funct, instruction);
			running = false;
			// Handle unknown instruction
			break;
	}
	
}

void CPU::exception(Exception except) {
	printf("Doing an exception : %08x", except);
	uint32_t handler = (sr & (1 << 22)) ? 0xbfc00180 : 0x80000080;
	auto mode = sr & 0x3F;
	sr &= ~0x3f;
	sr |= (mode << 2) & 0x3F;

	cause = (static_cast<uint32_t>(except) << 2);
	epc = current_pc;
	if (delay_slot) {
		epc -= 4;
		cause |= (1 << 31);
	}
	pc = handler;
	next_pc = pc + 4;
}

void CPU::op_syscall(Instruction instruct) {
	exception(Exception::SysCall);
}
void CPU::op_break(Instruction instruct) {
	exception(Exception::Break);
}
void CPU::op_cop0(Instruction instruct) {
	switch (instruct.rs) {
	case 0b10000:
		op_rfe(instruct);
		break;
	case 0b00100:
		op_mtc0(instruct);
		break;
	case 0000000:
		op_mfc0(instruct);
		break;
	default:
		printf("Unknown cop instruction: 0x%08X\n", instruct.rs);
		break;
	}
}
void CPU::op_cop1(Instruction instruct) {
	exception(Exception::CoprocessorError);
}
void CPU::op_cop2(Instruction instruct) {
	printf("Unhandled cop2");
}
void CPU::op_cop3(Instruction instruct) {
	exception(Exception::CoprocessorError);
}


uint32_t CPU::read32(uint32_t address) {
	return CPU::interconnect->read32(address); // Read a 32-bit value from the interconnect
}
uint8_t CPU::read8(uint32_t addr) {
	return interconnect->read8(addr);
}
void CPU::write32(uint32_t address, uint32_t value) {

	CPU::interconnect->write32(address, value);

}

void CPU::write8(uint32_t address, uint8_t value) {
	CPU::interconnect->write8(address, value);
}
void CPU::set_register(uint32_t reg, uint32_t value) {
	out_regs[reg] = value; // Set the register to the given value
	out_regs[0] = 0; // Ensure $zero register is always 0
	/*regs[reg] = value;
	regs[0] = 0;*/
}


void CPU::op_rfe(Instruction instruct) {
	if ((instruct.opcode & 0x3F) != 0b010000) {
		printf("Invalid cop0 instruction rfe");
	}
	auto mode = sr & 0x3F;
	sr &= ~0x3F;
	sr |= mode >> 2;
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
	auto v = regs[rs] | i; // Perform OR operation with the immediate value
	set_register(rt, v); // Set the target register with the result
}
void CPU::op_sw(Instruction instruct) {
	if ((sr & 0x10000) != 0) {
		//printf("Ignoring store while cache is isolated");
		return;
	}
	
	auto i_se = instruct.immediate_sign_extended;
	auto t = instruct.rt;
	auto s = instruct.rs;
	uint32_t addr = regs[s] + i_se;
	uint32_t v = regs[t];
	if (addr % 4 == 0) {
		
		write32(addr, v);
		
	}
	else {
		printf("Problem here1");
		exception(Exception::StoreAddressError);
	}
}
void CPU::op_sll(Instruction instruct) {
	auto i = instruct.shamt;
	auto t = instruct.rt;
	auto d = instruct.rd;
	auto v = regs[t] << i;
	set_register(d, v);
}
void CPU::op_addiu(Instruction instruct) {
	int32_t imm_se = instruct.immediate_sign_extended;
	auto t = instruct.rt;
	auto s = instruct.rs;
	//printf("T: %08x, S: %08x, ImmSE: %08x", t, s, imm_se);
	int32_t res = int32_t(regs[s]) + imm_se; 
	set_register(t, uint32_t(res));
	
}
void CPU::op_jmp(Instruction instruct) {
	auto j = instruct.jump_addr;
	next_pc = (pc & 0xF0000000) | ((j << 2) & 0x0FFFFFFC);
	in_branch = true;
}
void CPU::op_or(Instruction instruct) {
	auto d = instruct.rd;
	auto s = instruct.rs;
	auto t = instruct.rt;

	auto v = regs[s] | regs[t];
	set_register(d, v);
}
void CPU::op_addi(Instruction instruct) {
	int32_t imm_se = instruct.immediate_sign_extended;
	auto t = instruct.rt;
	auto s = instruct.rs;

	int32_t s2 = regs[s];

	
	auto v = checked_add_signed(s2, imm_se);
	if (v.has_value()) {
		set_register(t, v.value());
	}
	else {
		exception(Exception::Overflow);
	}
	
}
void CPU::op_lw(Instruction instruct) {

	if ((sr & 0x10000) != 0) {
		printf("\nCant lw, the cache is isolated");
		return;
	}

	auto imm_se = instruct.immediate_sign_extended;
	auto t = instruct.rt;
	auto s = instruct.rs;
	auto addr = regs[s] + imm_se;
	if (addr % 4 == 0) {
		auto v = read32(addr);
		//printf("\nLoading 1");
		load.first =t;
		load.second = v;
	}
	else {
		exception(Exception::LoadAddressError);
	}
}
void CPU::op_sltu(Instruction instruct) {
	auto s = instruct.rs;
	auto t = instruct.rt;
	auto d = instruct.rd;
	set_register(d, uint32_t(regs[s] < regs[t]));
}
void CPU::op_addu(Instruction instruct) {
	auto s = instruct.rs;
	auto t = instruct.rt;
	auto d = instruct.rd;
	auto v = regs[s] + regs[t];
	set_register(d, v);
}

void CPU::op_sh(Instruction instruct) {
	if ((sr & 0x10000) != 0) {
		printf("Ignoring store while cache is isolated");
		return;
	}
	auto imm_se = instruct.immediate_sign_extended;
	auto t = instruct.rt;
	auto s = instruct.rs;
	uint32_t add = regs[s] + imm_se;
	uint16_t v = static_cast<uint16_t>(regs[t]);
	if (add % 2 == 0) {
		write16(add, v);
	}
	else {
		printf("Problem here2");
		exception(Exception::StoreAddressError);
	}
	
}
void CPU::op_jal(Instruction instruct) {
	set_register(31, next_pc);
	op_jmp(instruct);
	in_branch = true;
}
void CPU::op_andi(Instruction instruct) {
	auto imm = instruct.immediate;
	auto t = instruct.rt;
	auto s = instruct.rs;
	auto v = regs[s] & imm;
	set_register(t, v);
}
void CPU::op_sb(Instruction instruct) {
	if ((sr & 0x10000) != 0) {
		printf("Ignore store while cache is isolated");
		return;
	}
	auto imm_se = instruct.immediate_sign_extended;
	auto t = instruct.rt;
	auto s = instruct.rs;
	auto add = regs[s] + imm_se;
	uint8_t v = regs[t];
	write8(add, v);
}
void CPU::op_jr(Instruction instruct) {
	next_pc = regs[instruct.rs];
	in_branch = true;
}
void CPU::op_lb(Instruction instruct) {
	auto imm_se = instruct.immediate_sign_extended;
	auto t = instruct.rt;
	auto s = instruct.rs;
	auto add = regs[s] + imm_se;
	int8_t v = read8(add);
	//printf("\nloading 2");
	load.first = t;
	load.second = uint32_t(v);
}
void CPU::op_beq(Instruction instruct) {
	auto imm_se = instruct.immediate_sign_extended;
	auto s = instruct.rs;
	auto t = instruct.rt;
	if (regs[s] == regs[t]) {
		branch(imm_se);
	}
}

void CPU::op_mfc0(Instruction instruct) {
	auto cpu_r = instruct.rt;
	auto cop_r = instruct.rd; //Not a register index
	uint32_t v = NULL;
	switch (cop_r) {
	case 14:
		v = epc;
		break;
	case 13:
		v = cause;
		break;
	case 12:
		v = sr;
		break;
	}
	//printf("\nload 3");
	load.first = cpu_r;
	load.second = v;

	
}

void CPU::op_mtc0(Instruction instruct) {
	auto cpu_r = instruct.rt;
	auto cop_r = instruct.rd; //Not a register index

	auto v = regs[cpu_r];
	switch (cop_r) {
	case 3:
	case 5:
	case 6:
	case 7:
	case 9:
	case 11:
		if (v != 0) { // Breakpoint fallthrough
			printf("Unhandled Write to cop0r\n");
			break;
		}
		break;
	case 12 :{
		sr = v;
		break;
		}
	case 13 : {
		cause = v;
		break;
	}
	default:
		printf("unhandle cop0 index instruct");
		break;
	}
}
void CPU::op_and(Instruction instruct) {
	auto d = instruct.rd;
	auto s = instruct.rs;
	auto t = instruct.rt;
	auto v = regs[s] & regs[t];
	set_register(d, v);
}
void CPU::op_add(Instruction instruct) {
	auto s = static_cast<int32_t>(regs[instruct.rs]);
	auto d = instruct.rd;
	auto t = static_cast<int32_t>(regs[instruct.rt]);

	auto v = checked_add_signed(s, t);
	if (v.has_value()) {
		set_register(d, v.value());
	}
	else {
		exception(Exception::Overflow);
	}

}
void CPU::op_bgtz(Instruction instruct) {
	auto imm_se = instruct.immediate_sign_extended;
	auto s = instruct.rs;
	int32_t v = regs[s];
	if (v > 0) {
		branch(imm_se);
	}
}

void CPU::op_bne(Instruction instruct) {
	auto imm_se = instruct.immediate_sign_extended;
	auto s = instruct.rs;
	auto t = instruct.rt;
	if (regs[s] != regs[t]) {
		branch(imm_se);
	}

}

void CPU::op_blez(Instruction instruct) {
	auto imm_se = instruct.immediate_sign_extended;
	auto s = instruct.rs;
	int32_t v = regs[s];
	if (v <= 0) {
		branch(imm_se);
	}
}
void CPU::op_lbu(Instruction instruct) {
	auto imm_se = instruct.immediate_sign_extended;
	auto s = instruct.rs;
	auto t = instruct.rt;
	auto add = regs[s] + imm_se;
	auto v = read8(add);
	//printf("\nLoad 4");
	load.first = t;
	load.second = uint32_t(v);
}
void CPU::op_jalr(Instruction instruct) {
	auto d = instruct.rd;
	auto s = instruct.rs;
	auto ra = next_pc;
	next_pc = regs[s];
	set_register(d, ra);
	in_branch = true;
}
void CPU::op_bxx(Instruction instruct) {
	auto imm_se = instruct.immediate_sign_extended;
	auto s = instruct.rs;
	auto instruction = instruct.opcode;

	uint32_t is_bgez = (instruction >> 16) & 1;
	auto is_link = ((instruction >> 17) & 0xf) == 8;

	int32_t v = regs[s];

	uint32_t test = (v < 0);

	test = test ^ is_bgez;
	if (is_link) {
		auto ra = next_pc;
		set_register(31, ra);
	}
	if( test != 0){
		branch(imm_se);
	}
}
void CPU::op_slti(Instruction instruct) {
	int32_t imm_se = instruct.immediate_sign_extended;
	auto s = instruct.rs;
	auto t = instruct.rt;
	uint32_t v = (int32_t(regs[s]) < imm_se);
	set_register(t, v);
}
void CPU::op_subu(Instruction instruct) {
	auto d = instruct.rd;
	auto s = instruct.rs;
	auto t = instruct.rt;
	uint32_t v = regs[s] - regs[t];
	set_register(d, v);
}
void CPU::op_sub(Instruction instruct) {
	auto d = instruct.rd;
	auto s = instruct.rs;
	auto t = instruct.rt;
	int32_t s_i = regs[s];
	int32_t t_i = regs[t];
	if (auto v = checked_sub_signed(s_i, t_i); v.has_value()) {
		set_register(d, v.value());
	}
	else {
		exception(Exception::Overflow);
	}
}
void CPU::op_sra(Instruction instruct) {
	auto shift = instruct.shamt;
	auto d = instruct.rd;
	auto t = instruct.rt;
	uint32_t v = int32_t(regs[t]) >> shift;
	set_register(d, v);
}
void CPU::op_div(Instruction instruct) {
	auto s = instruct.rs;
	auto t = instruct.rt;
	int32_t n = regs[s];
	int32_t d = regs[t];
	if (d == 0) {
		hi = n;
		if (n >= 0) {
			lo = 0xFFFFFFFF;
		}
		else {
			lo = 1;
		}
	}
	else if (uint32_t(n) == 0x80000000 && d == -1) {
		hi = 0;
		lo = 0x80000000;

	}
	else {
		hi = uint32_t(n % d);
		lo = uint32_t(n / d);
	}
}

void CPU::op_srl(Instruction instruct) {
	auto shift = instruct.shamt;
	auto d = instruct.rd;
	auto t = instruct.rt;

	auto v = regs[t] >> shift;
	set_register(d, v);
}
void CPU::op_sltiu(Instruction instruct) {
	auto imm_se = instruct.immediate_sign_extended;
	auto s = instruct.rs;
	auto t = instruct.rt;
	uint32_t v = regs[s] < imm_se;

	set_register(t, v);
}
void CPU::op_divu(Instruction instruct) {
	auto s = instruct.rs;
	auto t = instruct.rt;

	auto n = regs[s];
	auto d = regs[t];
	if (d == 0) {
		hi = n;
		lo = 0xFFFFFFFF;
	}
	else {
		hi = n % d;
		lo = n / d;

	}
}
void CPU::op_mflo(Instruction instruct) {
	auto d = instruct.rd;
	set_register(d, lo);
}
void CPU::op_mfhi(Instruction instruct) {
	auto d = instruct.rd;
	set_register(d, hi);
}
void CPU::op_slt(Instruction instruct) {
	uint32_t d = instruct.rd;
	uint32_t s = instruct.rs;
	uint32_t t = instruct.rt;
	int32_t si = regs[s];
	int32_t ti = regs[t];
	set_register(d, uint32_t(si < ti));
}

void CPU::op_mtlo(Instruction instruct) {
	auto s = instruct.rs;
	lo = regs[s];
}
void CPU::op_mthi(Instruction instruct) {
	auto s = instruct.rs;
	hi = regs[s];
}
void CPU::op_lhu(Instruction instruct) {
	auto imm_se = instruct.immediate_sign_extended;
	auto s = instruct.rs;
	auto t = instruct.rt;
	uint32_t add = regs[s] + imm_se;

	if ((add % 2) == 0) {
		//printf("\nload 1313");
		load.first = t;
		load.second = uint32_t(uint16_t(read16(add)));
		
	}
	else {
		exception(Exception::LoadAddressError);
	}

}
void CPU::op_sllv(Instruction instruct) {
	auto d = instruct.rd;
	auto s = instruct.rs;
	auto t = instruct.rt;
	auto v = regs[t] << (regs[s] & 0x1F);
	set_register(d, v);
}
void CPU::op_lh(Instruction instruct) {
	auto imm_se = instruct.immediate_sign_extended;
	auto s = instruct.rs;
	auto t = instruct.rt;
	auto add = regs[s] + imm_se;
	if (add % 2 == 0) {
		//printf("\ncaca 22 33 ");
		load.first = t;
		load.second = uint32_t(int16_t(read16(add)));
		
	}
	else {
		exception(Exception::LoadAddressError);
	}
}
void CPU::op_nor(Instruction instruct) {
	auto d = instruct.rd;
	auto s = instruct.rs;
	auto t = instruct.rt;
	auto v = ~(regs[s] | regs[t]);
	set_register(d, v);
}
void CPU::op_srav(Instruction instruct) {
	auto d = instruct.rd;
	auto s = instruct.rs;
	auto t = instruct.rt;
	uint32_t v = int32_t(regs[t]) >> (regs[s] & 0x1F);
	set_register(d, v);
}
void CPU::op_srlv(Instruction instruct) {
	auto d = instruct.rd;
	auto s = instruct.rs;
	auto t = instruct.rt;
	auto v = regs[t] >> (regs[s] & 0x1F);
	set_register(d, v);

}
void CPU::op_multu(Instruction instruct) {
	auto s = instruct.rs;
	auto t = instruct.rt;

	uint64_t a = regs[s];
	uint64_t b = regs[t];

	auto v = a * b;

	hi = uint32_t(v >> 32);
	lo = uint32_t(v);
}
void CPU::op_xor(Instruction instruct) {
	auto d = instruct.rd;
	auto s = instruct.rs;
	auto t = instruct.rt;
	auto v = regs[s] ^ regs[t];
	set_register(d, v);
}
void CPU::op_mult(Instruction instruct) {
	auto s = instruct.rs;
	auto t = instruct.rt;

	int64_t a = int32_t(regs[s]);
	int64_t b = int32_t(regs[t]);

	uint64_t v = a * b;

	hi = uint32_t(v >> 32);
	lo = uint32_t(v);
}

void CPU::op_xori(Instruction instruct) {
	auto i = instruct.immediate;
	auto s = instruct.rs;
	auto t = instruct.rt;
	set_register(t,  regs[s]^ i);
}
void CPU::op_lwl(Instruction instruct) {
	uint32_t i = instruct.immediate_sign_extended;  // Sign-extended immediate
	uint32_t t = instruct.rt;      // Target register index
	uint32_t s = instruct.rs;      // Source register index

	uint32_t addr = regs[s] + i;

	// This instruction bypasses the usual load delay restriction:
	// If there is a value already being loaded, it merges with that.
	uint32_t curv = out_regs[t];

	// Load the aligned word (address rounded down to multiple of 4)
	uint32_t aligned_addr = addr & ~3;
	uint32_t aligned_word = read32(aligned_addr);

	// Merge bytes depending on alignment (little endian assumed)
	uint32_t v = 0;
	switch (addr & 3) {
	case 0:
		v = (curv & 0x00FFFFFF) | (aligned_word << 24);
		break;
	case 1:
		v = (curv & 0x0000FFFF) | (aligned_word << 16);
		break;
	case 2:
		v = (curv & 0x000000FF) | (aligned_word << 8);
		break;
	case 3:
		v = (curv & 0x00000000) | (aligned_word << 0);
		break;
	default:
		throw std::runtime_error("Unreachable case in LWL");
	}
	//printf("\nomafaaa");
	load.first =t;
	load.second = v;
}

void CPU::op_lwr(Instruction instruct) {
	int32_t i = instruct.immediate_sign_extended;  // Sign-extended immediate
	uint32_t t = instruct.rt;      // Target register index
	uint32_t s = instruct.rs;      // Source register index

	uint32_t addr = regs[s] + i;

	// This instruction bypasses the usual load delay restriction:
	// If there is a value already being loaded, it merges with that.
	uint32_t curv = out_regs[t];

	// Load the aligned word (address rounded down to multiple of 4)
	uint32_t aligned_addr = addr & ~3;
	uint32_t aligned_word = read32(aligned_addr);

	// Merge bytes depending on alignment (little endian assumed)
	uint32_t v = 0;
	switch (addr & 3) {
	case 0: v = (curv & 0x00000000) | (aligned_word >> 0); break;
	case 1:
		v = (curv & 0xFF000000) | (aligned_word >> 8);
		break;
	case 2:
		v = (curv & 0xFFFF0000) | (aligned_word >> 16);
		break;
	case 3:
		v = (curv & 0xFFFFFF00) | (aligned_word >> 24);
		break;
	default:
		throw std::runtime_error("Unreachable alignment case in LWR");
	}
	//printf("\nomamaaa");
	load.first = t;
	load.second = v;
}

void CPU::op_swl(Instruction instruct) {
	uint32_t i = instruct.immediate_sign_extended;
	uint32_t t = instruct.rt;
	uint32_t s = instruct.rs;

	uint32_t addr = regs[s] + i;
	uint32_t v = regs[t];

	uint32_t aligned_addr = addr & ~0x3;
	uint32_t cur_mem = read32(aligned_addr);
	uint32_t mem;

	switch (addr & 3) {
	case 0:
		mem = (cur_mem & 0xFFFFFF00) | (v >> 24);
		break;
	case 1:
		mem = (cur_mem & 0xFFFF0000) | (v >> 16);
		break;
	case 2:
		mem = (cur_mem & 0xFF000000) | (v >> 8);
		break;
	case 3:
		mem = v;
		break;
	default:
		throw std::runtime_error("Unreachable SWL case");
	}

	write32(aligned_addr, mem);

}
void CPU::op_swr(Instruction instruct) {
	uint32_t i = instruct.immediate_sign_extended;
	uint32_t t = instruct.rt;
	uint32_t s = instruct.rs;

	uint32_t addr = regs[s] + i;
	uint32_t v = regs[t];

	uint32_t aligned_addr = addr & ~0x3;
	uint32_t cur_mem = read32(aligned_addr);
	uint32_t mem;

	switch (addr & 3) {
	case 0:
		mem = (cur_mem & 0x00000000) | (v << 0);
		break;
	case 1:
		mem = (cur_mem & 0x000000FF) | (v << 8);
		break;
	case 2:
		mem = (cur_mem & 0x0000FFFF) | (v << 16);
		break;
	case 3:
		mem = (cur_mem & 0x00FFFFFF) | (v << 24);
		break;
	default:
		throw std::runtime_error("Unreachable SWR case");
	}

	write32(aligned_addr, mem);
}