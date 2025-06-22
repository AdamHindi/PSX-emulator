#include <cstdint>
#include <vector>


class RAM {
	
public:
	RAM() : dataRam(2 * 1024 * 1024, 0xCA) {}
	std::vector<uint8_t> dataRam;
	void reset();
	uint32_t read32(uint32_t offset);
	void write32(uint32_t offset, uint32_t value);
	uint16_t read16(uint32_t offset);
	void write16(uint32_t offset, uint16_t value);

	uint8_t read8(uint32_t offset);
	void write8(uint32_t offset, uint8_t value);
}; 


