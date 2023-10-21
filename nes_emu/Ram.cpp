#include "Ram.h"

uint8_t Ram::read(uint16_t address)
{
	return mem[address];
}

void Ram::write(uint16_t address, uint8_t val)
{
	mem[address] = val;
}
