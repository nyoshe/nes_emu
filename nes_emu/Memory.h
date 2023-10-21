#pragma once
#include <cstdint>


class Memory
{
public:
	//peek used for debug, don't notify we read a value
	virtual uint8_t peek(uint16_t address) { return 0; };
	virtual uint8_t read(uint16_t address) { return 0; };
	virtual void write(uint16_t address, uint8_t val){};
};


struct Mapping
{
	uint16_t start_pos;
	uint16_t end_pos;
	Memory* loc;

	Mapping(Memory* l, uint16_t s, uint16_t e)
	{
		loc = l;
		start_pos = s;
		end_pos = e;
	}
};