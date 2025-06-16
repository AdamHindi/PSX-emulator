#include "bios.hpp"  
#include <fstream> // Include the header for std::ifstream  

void BIOS::init(const std::string& path) {  
    // Initialize the BIOS  
    // This function can be expanded to include more initialization code as needed  
    std::cout << "Initializing BIOS from path: " << path << std::endl;  

    // Load BIOS to BIOS_data from path:  
    std::ifstream file(path, std::ios::binary | std::ios::ate);  
    if (!file) {  
        std::cerr << "Failed to open BIOS file: " << path << std::endl;  
        return;  
    }  

    // Initialize biosData with the file size  
    std::streamsize size = file.tellg();  
    if (size != 512 * 1024) {
        std::cerr << "BIOS file not 512KB." << std::endl;
        return;
    }

    file.seekg(0, std::ios::beg);  

    biosData.resize(size); // Resize the vector to hold the file data 
    
    if (!file.read(reinterpret_cast<char*>(biosData.data()), size)) {  
        std::cerr << "Failed to read BIOS file: " << path << std::endl;  
    }  
    else {  
        std::cout << "BIOS loaded successfully." << std::endl;  
	}
}

uint32_t BIOS::read32(uint32_t address) const {  
    // Read a 32-bit value from the BIOS data at the specified address  
    uint32_t offset = address - 0xBFC00000;
	uint32_t b0 = biosData[offset];
	uint32_t b1 = biosData[offset + 1];
	uint32_t b2 = biosData[offset + 2];
	uint32_t b3 = biosData[offset + 3];
	// Combine the bytes into a 32-bit value
	return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24); // little-endian format
}

bool BIOS::rangeCheck(uint32_t address) const
{
    // BIOS is mapped from 0xBFC00000 to 0xBFC7FFFF (512KB)
    if (address >= 0xBFC00000 && address < 0xBFC00000 + 512 * 1024) {
        return true;
    }
    return false;
}
