#pragma once
#include <cstdint>
#include <vector>

#include "Bus.h"
#include "Memory.h"


class Ram : public Memory
{
	uint8_t mem[0x0800];
	Bus* bus;
public:
	Ram(Bus* _bus)
	{
		bus = _bus;
		bus->add_map(this, 0x2000, 0x0800);
	}
	uint8_t read(uint16_t address) override;
	uint8_t peek(uint16_t address) override { return mem[address]; };
	void write(uint16_t address, uint8_t val) override;
};

