#include <cstdint>

#include <iostream>
#include <vector>


// BIOS interface for the BIOS class
class BIOS {
public:
	BIOS(const std::string& path) {
		init(path);
	}

public:
	void init(const std::string& path); // load BIOS from BIOS file
	uint32_t read32(uint32_t address) const; // read a byte from the BIOS at the specified address
	bool rangeCheck(uint32_t address) const; // check if the address is within the BIOS range
private:
	std::vector<uint8_t> biosData; //BIOS data

};

