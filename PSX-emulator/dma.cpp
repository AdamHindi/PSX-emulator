#include "dma.hpp"
#include <stdexcept>


bool DMA::irq() {
	auto channel_irq = channel_irq_en & channel_irq_flags;
	return force_irq || (irq_en && channel_irq != 0);
}

uint32_t DMA::interrupt() {
	uint32_t r = 0;
	r |= uint32_t( irq_dummy);
	r |= (uint32_t(force_irq)) << 15;
	r |= uint32_t(channel_irq_en) << 16;
	r |= uint32_t(irq_en) << 23;
	r |= uint32_t(channel_irq_flags) << 24;
	r |= uint32_t(irq()) << 31;
	return r;
}

void DMA::set_interrupt(uint32_t val) {
	irq_dummy = uint8_t(val & 0x3f);
	force_irq = ((val >> 15) & 1) != 0;
	channel_irq_en = uint8_t((val >> 16) & 0x7F);
	irq_en = ((val >> 23) & 1) != 0;
	auto ack = uint8_t((val >> 24) & 0x3f);
	channel_irq_flags &= !ack;
}

uint8_t Channel::get_control() const{
	uint32_t r = 0;
	r |= uint32_t(direction) <<0;
	r |= uint32_t(step) << 1;
	r |= uint32_t(chop) << 8;
	r |= uint32_t(sync) << 9;
	r |= uint32_t(chop_dma_sz) << 16;
	r |= uint32_t(enable) << 24;
	r |= uint32_t(trigger) << 28;
	r |= uint32_t(dummy) << 29;
	return r;
}
void Channel::set_control(uint32_t val) {
	direction = ((val & 1) !=0) ? Direction::FromRam : Direction::ToRam;

	step = (((val >> 1) & 1) !=0) ? Step::Dec : Step::Inc;

	chop = ((val >> 8) & 1) != 0;

	uint32_t sync_mode = (val >> 9) & 0b11;
	switch (sync_mode) {
	case 0: sync = Sync::Manual; break;
	case 1: sync = Sync::Request; break;
	case 2: sync = Sync::LinkedList; break;
	default:
		throw std::runtime_error("Unknown DMA sync mode: ");
	}

	chop_dma_sz = static_cast<uint8_t>((val >> 16) & 0x7);
	chop_cpu_sz = static_cast<uint8_t>((val >> 20) & 0x7);

	enable = ((val >> 24) & 1) != 0;
	trigger = ((val >> 28) & 1) != 0;
	dummy = static_cast<uint8_t>((val >> 29) & 3);
}