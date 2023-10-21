#include "Bus.h"
void Bus::add_map(Memory* loc, uint16_t start, uint16_t size)
{
	mapped_loc.emplace_back(loc, start, start + size - 1);
}


uint8_t Bus::peek(uint16_t address)
{
	for (auto m : mapped_loc) {
		if (address >= m.start_pos && address <= m.end_pos) {
			return m.loc->peek(address - m.start_pos);
		}
	}
}

uint8_t Bus::peek(uint8_t ADH, uint8_t ADL)
{
	return peek((static_cast<uint16_t>(ADH) << 8) | ADL);
}

uint8_t Bus::read(uint16_t address)
{
	for (auto m : mapped_loc) {
		if (address >= m.start_pos && address <= m.end_pos) {
			return m.loc->read(address - m.start_pos);
		}
	}
}

uint8_t Bus::read(uint8_t ADH, uint8_t ADL)
{
	return read((static_cast<uint16_t>(ADH) << 8) | ADL);
}

void Bus::write(uint16_t address, uint8_t val)
{
	for (auto m : mapped_loc) {
		if (address >= m.start_pos && address <= m.end_pos) {
			m.loc->write(address - m.start_pos, val);
		}
	}
}

void Bus::write(uint8_t ADH, uint8_t ADL, uint8_t val)
{
	write((static_cast<uint16_t>(ADH) << 8) | ADL, val);
}


