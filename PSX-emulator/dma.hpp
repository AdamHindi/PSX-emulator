#pragma once

#include <cstdint>
#include <stdio.h>
#include <optional>
#include <stdexcept>
enum class Port : uint32_t {
	MdecIn = 0,
	MdecOut = 1,
	Gpu = 2,
	CdRom = 3,
	Spu= 4,
	Pio = 5,
	Otc =6,

};
enum Direction {
	ToRam = 0,
	FromRam = 1
};

enum Step {
	Inc = 0,
	Dec = 1
};

enum Sync {
	Manual = 0,
	Request = 1,
	LinkedList = 2
};
inline Port port_from_index(uint32_t index) {
	switch (index) {
	case 0: return Port::MdecIn;
	case 1: return Port::MdecOut;
	case 2: return Port::Gpu;
	case 3: return Port::CdRom;
	case 4: return Port::Spu;
	case 5: return Port::Pio;
	case 6: return Port::Otc;
	default:
		printf("Invalid DMA Port");
		std::abort();
	}
};
class Channel {
public:
	Channel() {
		enable = false;
		direction = Direction::ToRam;
		step = Step::Inc;
		sync = Sync::Manual;
		trigger = false;
		chop = false;
		chop_dma_sz = 0;
		chop_cpu_sz = 0;
		dummy = 0;
		base = 0;
	}
public:
	bool active() {
		bool t = (sync == Sync::Manual) ? trigger : true;
		return enable && t;
	}
	void done() {
		enable = false;
		trigger = false;
	}
	Sync get_sync() const {
		return sync;
	};
	Direction get_direction() const {
		return direction;
	}

	Step get_step()const {
		return step;
	}
	uint32_t get_control() const;
	void set_control(uint32_t value);
	uint32_t get_base() const { return base; };
	void set_base(uint32_t value) { base = value & 0xffffff; };
	uint32_t block_control() const {
		return (static_cast<uint32_t>(block_count) << 16) |
			static_cast<uint32_t>(block_size);
	};
	void set_block_control(uint32_t val) {
		block_size = static_cast<uint16_t>(val);
		block_count = static_cast<uint16_t>(val >> 16);
	}

	std::optional<uint32_t> transfer_size() const {
		uint32_t bs = static_cast<uint32_t>(block_size);
		uint32_t bc = static_cast<uint32_t>(block_count);

		switch (sync) {
		case Sync::Manual:
			return bs;
		case Sync::Request:
			return bc * bs;
		case Sync::LinkedList:
			return std::nullopt;
		default:
			throw std::runtime_error("Unknown DMA sync mode");
		}
	}
private:
	bool enable;
	Direction direction;
	Step step;
	Sync sync;
	bool trigger;
	bool chop;
	uint8_t chop_dma_sz; // Chopping dma window size 
	uint8_t chop_cpu_sz;
	uint8_t dummy;
	uint32_t base;//address of first word to be read or written in ram
	uint16_t block_size = 0;
	uint16_t block_count = 0;
};

class DMA {
public:
	DMA() : channels {}  {
		reset();
	};
	uint32_t get_control() const {
		return control;
	};
	void set_control(uint32_t value) {
		control = value;
	};
	void reset() {
		control = 0x07654321;
		irq_en = false;
		channel_irq_en = 0;
		channel_irq_flags = 0;
		force_irq = false;
		irq_dummy = 0;
	};
	void done(Port port)
	{
		channels[(size_t)port].done();          // clear enable/trigger

		uint8_t mask = 1 << static_cast<uint8_t>(port);
		channel_irq_flags |= (channel_irq_en & mask);   // raise flag
		/* if you later wire the global IRQ controller,
		   also check irq() and assert CPU IRQ here */
	}
	bool irq();
	uint32_t interrupt();
	void set_interrupt(uint32_t value);
	Channel& channel(Port port) { return channels[static_cast<size_t>(port)]; };
	uint8_t channel_irq_flags;

private:
	Channel channels[7];
	uint32_t control;
	bool irq_en;
	uint8_t channel_irq_en;
	
	bool force_irq;
	uint8_t irq_dummy;


};
