#include "bios.hpp"


class Interconnect {

public:
	Interconnect(BIOS b) : bios(b) {};
	
	uint32_t read32(uint32_t address) {
		// Simulate loading a 32-bit value from the specified address
		// In a real implementation, this would access memory or an I/O port
		// To be made into a switch statement later
		if (address % 4 != 0) {
			printf("Address 0x%08X is not aligned to 4 bytes.\n", address);
			return 0; // Return 0 if address is not aligned
		}
		// Check if the address is within the range of the BIOS
		if (bios.rangeCheck(address)) {
			return bios.read32(address);
		}
		printf("Unhandled read, 0x%08X is out of range.\n", address);
		return 0; // Return 0 if address is out of range
	}
	void write32(uint32_t address, uint32_t value) {
		// Simulate writing a 32-bit value to the specified address
		// In a real implementation, this would access memory or an I/O port
		if (address % 4 != 0) {
			printf("Address 0x%08X is not aligned to 4 bytes.\n", address);
			return; // Do nothing if address is not aligned
		}
		printf("Unhandled write, 0x%08X is out of range.\n", address);
	}
private:
	BIOS bios;
};
