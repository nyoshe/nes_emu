#pragma once
#include <cstdint>
#include <vector>

#include "Memory.h"

class Bus
{
	std::vector<Mapping> mapped_loc;
	
public:
	Bus() = default;
	//~Bus() { mapped_loc.clear(); }
	bool IRQ = false;
	bool NMI = false;
	void add_map(Memory* loc, uint16_t start, uint16_t size);
	uint8_t peek(uint16_t address);
	uint8_t peek(uint8_t ADH, uint8_t ADL);
	uint8_t read(uint16_t address);
	uint8_t read(uint8_t ADH, uint8_t ADL);
	void write(uint16_t address, uint8_t val);
	void write(uint8_t ADH, uint8_t ADL, uint8_t val);
	void step();
};

