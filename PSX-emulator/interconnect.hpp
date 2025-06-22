#include "bios.hpp"
#include <optional>
#include "ram.hpp"
#include "dma.hpp"

constexpr uint32_t REGION_MASK[8] = {
	// KUSEG: 2048MB
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	// KSEG0: 512MB
	0x7FFFFFFF,
	// KSEG1: 512MB
	0x1FFFFFFF,
	// KSEG2: 1024MB
	0xFFFFFFFF, 0xFFFFFFFF
};



inline uint32_t mask_region(uint32_t addr) {
	uint32_t index = addr >> 29; // top 3 bits -> 8 possible regions
	return addr & REGION_MASK[index];
}


struct Range {
	uint32_t start;
	uint32_t length;

	// Returns offset if addr is in range, or std::nullopt
	std::optional<uint32_t> contains(uint32_t addr) const {
		if (addr >= start && addr < start + length) {
			return addr - start;
		}
		return std::nullopt;
	}
};

class Interconnect {

public:
	Interconnect(const BIOS& b, const RAM& r, const DMA& d) : bios(b), ram(r), dma(d) {};
	
	uint32_t read32(uint32_t address);
	uint16_t read16(uint32_t address);
	// Read 8 bit value from memory
	uint8_t read8(uint32_t addr);

	void write32(uint32_t address, uint32_t value);
	void write16(uint32_t address, uint16_t value);
	void write8(uint32_t address, uint8_t value);
private:
	BIOS bios;
	RAM ram;
	DMA dma;
	uint32_t dma_reg(uint32_t offset);
	void set_dma_reg(uint32_t offset, uint32_t value);
	void do_dma(Port port);
	void do_dma_linked_list(Port port);
	void do_dma_block(Port port);

	static constexpr Range MEMCONTROL{ 0x1f801000, 36 };
	static constexpr Range RAM_RANGE{ 0x00000000, 8 * 1024 * 1024 };              // 2MB RAM
	static constexpr Range BIOS_RANGE{ 0x1FC00000, 512 * 1024 };                  // 512KB BIOS
	static constexpr Range SYS_CONTROL{ 0x1F801000, 36 };                   // unknown registers (mednafen)
	static constexpr Range RAM_SIZE{ 0x1F801060, 4 };                       // RAM config register
	static constexpr Range CACHE_CONTROL{ 0xfffe0130, 4 };                  // cache control (KSEG2)
	static constexpr Range SPU { 0x1F801C00 , 640 };
	static constexpr Range  EXPANSION_1 = { 0x1f000000, 512 * 1024 };
	static constexpr Range  EXPANSION_2 = { 0x1F802000, 66 };
	static constexpr Range  IRQ_CONTROL = { 0x1f801070, 8 };
	static constexpr Range DMA = { 0x1f801080, 0x80 };
	static constexpr Range TIMER = {0x1f801100, 0x30};
	static constexpr Range GPU_RANGE = { 0x1f801810, 8 };
};


