#include "interconnect.hpp"



void Interconnect::write32(uint32_t address, uint32_t value){
	// Simulate writing a 32-bit value to the specified address
	// In a real implementation, this would access memory or an I/O port
	if (address % 4 != 0) {
		printf("Address 0x%08X is not aligned to 4 bytes.\n", address);
		return; // Do nothing if address is not aligned
	}
	auto abs_adr = mask_region(address);
	if (auto offset = CACHE_CONTROL.contains(abs_adr)) {
		printf("\nUnhandled cache control write32");
		return;
	}
	if (auto offset = GPU_RANGE.contains(abs_adr); offset.has_value()) {
		printf("GPU WRITE 32");
		
		return;
	}
	if (auto offset = RAM_RANGE.contains(abs_adr)) {
		ram.write32(offset.value(), value);
		return;
	}
	if (auto offset = DMA.contains(abs_adr)) {
		set_dma_reg(offset.value(),value);
		return;
	}
	if (auto offset = IRQ_CONTROL.contains(abs_adr)) {
		printf("\nUnhandled IRQ Control\n");
		return;
	}
	if (auto offsetOpt = MEMCONTROL.contains(address)) {
		uint32_t offset = *offsetOpt;
		switch (offset) {
		case 0: // Expansion 1 base address
			if (value != 0x1f000000) {
				throw std::runtime_error("\nBad expansion 1 base address: 0x%32X" + value);
			}
			break;

		case 4: // Expansion 2 base address
			if (value != 0x1f802000) {
				throw std::runtime_error("\nBad expansion 2 base address: 0x%32X" + value);
			}
			break;

		default:
			std::printf("Unhandled write to MEMCONTROL register at offset 0x%x\n", offset);
			
			break;
		}
		return;
	}

	printf("Unhandled write, 0x%08X is out of range.\n", address);
}

uint16_t Interconnect::read16(uint32_t addr) {
	auto abs_addr = mask_region(addr);
	if (auto offset = SPU.contains(abs_addr); offset.has_value()) {
		//printf("\nUnhandled SPU read16\n");
		return 0;
	}
	if (auto offset = RAM_RANGE.contains(abs_addr); offset.has_value()) {
		return ram.read16(offset.value());
	}
	if (auto offset = IRQ_CONTROL.contains(abs_addr); offset.has_value()) {
		printf("IRQ READ \n");
		return 0;
	}
	printf("Unhandled Read 16 %08x", addr);
}

uint8_t Interconnect::read8(uint32_t addr) {
	auto abs_addr = mask_region(addr);
	if (auto offset = BIOS_RANGE.contains(abs_addr)) {
		return bios.read8(offset.value());
	}
	if (auto offset = EXPANSION_2.contains(abs_addr)) {
		return 0xff;
	}
	if (auto offset = RAM_RANGE.contains(abs_addr)) {
		return ram.read8(offset.value());
	}
	printf("Unhandled read in read8, 0x%08X is out of range.\n", addr);
}


uint32_t Interconnect::read32(uint32_t address){

	if (address % 4 != 0) {
		printf("Address 0x%08X is not aligned to 4 bytes.\n", address);
		return 0; // Return 0 if address is not aligned
	}
	auto abs_adr = mask_region(address);
	// Check if the address is within the range of the BIOS
	if (auto offset = BIOS_RANGE.contains(abs_adr)) {
		return bios.read32(offset.value());
	}
	if (auto offset = RAM_RANGE.contains(abs_adr)) {
		return ram.read32(offset.value());
	}
	if (auto offset = IRQ_CONTROL.contains(abs_adr)) {
		printf("IRQ control read\n");
		return 0;
	}
	if (auto offset = DMA.contains(abs_adr); offset.has_value()) {
		return dma_reg(offset.value());
	}
	if (auto offset = GPU_RANGE.contains(abs_adr); offset.has_value()) {
		printf("GPU READ \n");
		switch (offset.value()) {
		case(4):
			return 0x10000000;
		}
		return 0;
	}
	
	printf("Unhandled read, 0x%08X is out of range.\n", address);
	return 0; // Return 0 if address is out of range
}

void Interconnect::write16(uint32_t address, uint16_t val) {
	
		if (address % 2 != 0) {
			printf("Address 0x%08X is not aligned to 2 bytes.\n", address);
			return; // Do nothing if address is not aligned
		}
		auto abs_adr = mask_region(address);
		if (auto offset = TIMER.contains(abs_adr)) {
			printf("\nUnhandled timer write 16");
			return;
		}
		if (auto offset = IRQ_CONTROL.contains(abs_adr); offset.has_value()) {
			printf("IRQ WRITE \n");
			return ;
		}
		if (auto offset = SPU.contains(abs_adr)) {
			//printf("\nUnhandled SPU register %08x\n", offset.value());
			return;
		}
		if (auto offset = RAM_RANGE.contains(abs_adr); offset.has_value()) {
			ram.write16(offset.value(), val);
			return;
		}
		printf("\nUnhandled store, 0x%08X is out of range.\n", abs_adr);


}

void Interconnect::write8(uint32_t address, uint8_t value) {
	auto abs_addr = mask_region(address);
	if (auto offset = RAM_RANGE.contains(abs_addr)) {
		return ram.write8(offset.value(), value);
	}
	if (auto offset = EXPANSION_2.contains(abs_addr)) {
		printf("\nUnhandled write to expansion 2 reg");
		return;
	}
	printf("\nUNHANDLED STORE8 INTO ADD %08x", address);
}

void Interconnect::set_dma_reg(uint32_t offset, uint32_t val) {
	uint32_t major = (offset & 0x70) >> 4;
	uint32_t minor = offset & 0xF;

	std::optional<Port> active_port;

	if (major <= 6) {
		Port port = port_from_index(major);
		Channel& channel = dma.channel(port);

		switch (minor) {
		case 0x0: channel.set_base(val); break;
		case 0x4: channel.set_block_control(val); break;
		case 0x8: channel.set_control(val); break;
		default:
			throw std::runtime_error("Unhandled DMA write");
		}

		if (channel.active()) {
			active_port = port;
		}
	}
	else if (major == 7) {
		switch (minor) {
		case 4: dma.set_control(val);
		case 0: dma.set_interrupt(val);
		default:
			std::runtime_error("Unhandled DMA write");
		}
	}
	else {
		throw std::runtime_error("Unhandled DMA write");
	}

	if (active_port.has_value()) {
		do_dma(active_port.value());
	}
}


uint32_t Interconnect::dma_reg(uint32_t offset) {
	uint32_t align = offset & 3;
	uint32_t aligned_offset = offset & ~3;

	uint32_t major = (aligned_offset & 0x70) >> 4;
	uint32_t minor = aligned_offset & 0xf;

	uint32_t res;

	if (major <= 6) {
		auto& channel = dma.channel(port_from_index(major));
		switch (minor) {
		case 0x0: res = channel.get_base(); break;
		case 0x4: res = channel.block_control(); break;
		case 0x8: res = channel.get_control(); break;
		default: throw std::runtime_error("Unhandled DMA read");
		}
	}
	else if (major == 7) {
		switch (minor) {
		case 0x0: res = dma.get_control(); break;
		case 0x4: res = dma.interrupt(); break;
		default: throw std::runtime_error("Unhandled DMA read");
		}
	}
	else {
		throw std::runtime_error("Unhandled DMA read");
	}

	// Return only the requested byte/halfword if unaligned
	return res >> (align * 8);
}

void Interconnect::do_dma(Port port) {
	switch (dma.channel(port).get_sync()) {
	case Sync::LinkedList: do_dma_linked_list(port);break;
	default:
		do_dma_block(port);
		break;
	}
}
void Interconnect::do_dma_linked_list(Port port) {
	const Channel& channel = dma.channel(port);
	uint32_t addr = channel.get_base() & 0x1FFFFC;

	if (channel.get_direction() == Direction::ToRam) {
		throw std::runtime_error("Invalid DMA direction for linked list mode");
	}

	if (port != Port::Gpu) {
		throw std::runtime_error("Attempted linked list DMA on unsupported port");
	}

	while (true) {
		uint32_t header = ram.read32(addr);
		uint32_t remsz = header >> 24;

		while (remsz-- > 0) {
			addr = (addr + 4) & 0x1FFFFC;
			uint32_t command = ram.read32(addr);
			
		}

		if ((header & 0x800000)!=0) {
			break;
		}

		addr = header & 0x1FFFFC;
	}
}
void Interconnect::do_dma_block( Port port) {
	Channel& channel = dma.channel(port);

	int32_t step = (channel.get_step() == Step::Inc) ? 4 : -4;
	uint32_t addr = channel.get_base();

	std::optional<uint32_t> maybe_size = channel.transfer_size();
	if (!maybe_size.has_value()) {
		throw std::runtime_error("Couldn't determine DMA block transfer size");
	}
	uint32_t remsz = maybe_size.value();

	while (remsz-- > 0) {
		uint32_t cur_addr = addr & 0x1FFFFC;

		if (channel.get_direction() == Direction::FromRam) {
			uint32_t word = ram.read32(cur_addr);
			switch (port) {
				
				case Port::Spu:     /* TODO: implement SPU write */ break;
			default:
				throw std::runtime_error("Unhandled DMA destination port");
			}
		}
		else {
			uint32_t word;
			switch (port) {
			case Port::Otc:
				word = (remsz == 0) ? 0xFFFFFF : ((addr - 4) & 0x1FFFFF);
				break;
			case Port::Gpu:
				// GPU DMA to RAM (read): not implemented
				word = 0; // placeholder
				break;
			case Port::MdecOut:
				word = 0; // placeholder
				break;
			default:
				throw std::runtime_error("Unhandled DMA source port");
			}

			ram.write32(cur_addr, word);
		}

		addr += step;
	}
}
