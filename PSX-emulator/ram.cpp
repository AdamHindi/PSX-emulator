#include "ram.hpp"


uint32_t RAM::read32(uint32_t offset) {
	offset = static_cast<size_t>(offset);

	auto b0 = static_cast<uint32_t>(dataRam[offset + 0]);
	auto b1 = static_cast<uint32_t>(dataRam[offset + 1]);
	auto b2 = static_cast<uint32_t>(dataRam[offset + 2]);
	auto b3 = static_cast<uint32_t>(dataRam[offset + 3]);
	// Combine the bytes into a 32-bit value
	return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24); // little-endian format
}
uint16_t RAM::read16(uint32_t offset) {
	offset = static_cast<size_t>(offset);
	uint16_t b0 = dataRam[offset + 0];
	uint16_t b1 = dataRam[offset + 1];
	return b0 | (b1 << 8);
}
uint8_t RAM::read8(uint32_t offset) {
	return dataRam[offset];
}

void RAM::write32(uint32_t offset, uint32_t value) {
	offset = static_cast<size_t>(offset);

	auto b0 = static_cast<uint8_t>(value);
	auto b1 = static_cast<uint8_t>(value >> 8);
	auto b2 = static_cast<uint8_t>(value >> 16);
	auto b3 = static_cast<uint8_t>(value >> 24);
	dataRam[offset + 0] = b0;
	dataRam[offset + 1] = b1;
	dataRam[offset + 2] = b2;
	dataRam[offset + 3] = b3;

}
void RAM::write16(uint32_t offset, uint16_t val) {
	uint8_t b0 = val;
	uint8_t b1 = (val >> 8);

	dataRam[offset + 0] = b0;
	dataRam[offset + 1] = b1;

}

void RAM::write8(uint32_t offset, uint8_t val) {

	dataRam[offset] = val;
}