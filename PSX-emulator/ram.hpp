#include <cstdint>  
#include <vector>  

class RAM {  
public:  
    RAM() : dataRam(2 * 1024 * 1024), canary(0xDEADBEEF) {
    }
    // disable accidental moves/copies that leave a “zombie” RAM behind
    RAM(const RAM&) = delete;
    RAM& operator=(const RAM&) = delete;
    RAM(RAM&&) = delete;
    RAM& operator=(RAM&&) = delete;

    void check(const char* where) const {
        if (canary != 0xDEADBEEF) {
            std::fprintf(stderr, "[CANARY] smashed @ %s\n", where);
        }
    };

     
    void reset();
    uint32_t read32(uint32_t offset);
    void write32(uint32_t offset, uint32_t value);
    uint16_t read16(uint32_t offset);
    void write16(uint32_t offset, uint16_t value);

    uint8_t read8(uint32_t offset);
    void write8(uint32_t offset, uint8_t value);
private:
    uint32_t             canary;          // lives *right before* the vector
    std::vector<uint8_t> dataRam ;

};
