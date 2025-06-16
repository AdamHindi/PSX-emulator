#include <cstdint>



class CPU {

public:
		CPU() = default;
		~CPU() = default;
		void reset();
		void execute(uint32_t cycles);
		void interrupt(uint8_t vector);

private:

}




