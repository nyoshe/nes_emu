#pragma once
#include <cstdint>
#include <SFML/Graphics.hpp>
#include "Memory.h"
#include "Bus.h"
#include "imgui-sfml/imgui-SFML.h"
#include "imgui/imgui.h"
enum PPUREG
{
	PPUCTRL,
	PPUMASK,
	PPUSTATUS,
	OAMADDR,
	OAMDATA,
	PPUSCROLL,
	PPUADDR,
	PPUDATA,
};

enum TILE
{
	NT_BYTE,
	AT_BTYE,
	PT_LOW,
	PT_HIGH
};

struct TileInfo
{
	uint8_t bitplane1 = 0;
	uint8_t bitplane2 = 0;
	uint8_t palette = 0;
	uint16_t address = 0;
};
//ppu status info
const uint8_t SPR_OVERFLOW = 1 << 5;
const uint8_t SPR0_HIT = 1 << 6;
const uint8_t VBLANK = 1 << 7;

//ppu flags
const uint8_t NT_MASK = 0x03;
//how much to increment vram by
const uint8_t VRAM_INC = 1 << 2;
const uint8_t SPR_ADDR = 1 << 3;
const uint8_t BG_ADDR = 1 << 4;
const uint8_t SPR_SIZE = 1 << 5;
const uint8_t PPU_SELECT = 1 << 6;
const uint8_t DO_NMI = 1 << 7;

//ppu mask
const uint8_t GREYSCALE = 1 << 0;
//hides background and sprites for the leftmost 8 pixels
const uint8_t SHOW_BG_LEFT = 1 << 1;
const uint8_t SHOW_SPR_LEFT = 1 << 2;

//disables background and sprite rendering
const uint8_t SHOW_BG = 1 << 3;
const uint8_t SHOW_SPR = 1 << 4;

const uint8_t EMPHASIZE_RED = 1 << 5;
const uint8_t EMPHASIZE_GREEN = 1 << 6;
const uint8_t EMPHASIZE_BLUE = 1 << 7;

const uint32_t palette_ref[64] = {
0x7C7C7CFF,
0x0000FCFF,
0x0000BCFF,
0x4428BCFF,
0x940084FF,
0xA80020FF,
0xA81000FF,
0x881400FF,
0x503000FF,
0x007800FF,
0x006800FF,
0x005800FF,
0x004058FF,
0x000000FF,
0x000000FF,
0x000000FF,
0xBCBCBCFF,
0x0078F8FF,
0x0058F8FF,
0x6844FCFF,
0xD800CCFF,
0xE40058FF,
0xF83800FF,
0xE45C10FF,
0xAC7C00FF,
0x00B800FF,
0x00A800FF,
0x00A844FF,
0x008888FF,
0x000000FF,
0x000000FF,
0x000000FF,
0xF8F8F8FF,
0x3CBCFCFF,
0x6888FCFF,
0x9878F8FF,
0xF878F8FF,
0xF85898FF,
0xF87858FF,
0xFCA044FF,
0xF8B800FF,
0xB8F818FF,
0x58D854FF,
0x58F898FF,
0x00E8D8FF,
0x787878FF,
0x000000FF,
0x000000FF,
0xFCFCFCFF,
0xA4E4FCFF,
0xB8B8F8FF,
0xD8B8F8FF,
0xF8B8F8FF,
0xF8A4C0FF,
0xF0D0B0FF,
0xFCE0A8FF,
0xF8D878FF,
0xD8F878FF,
0xB8F8B8FF,
0xB8F8D8FF,
0x00FCFCFF,
0xF8D8F8FF,
0x000000FF,
0x000000FF,
};

class PPU : public Memory
{
private:
	uint8_t oam[0x100];
	uint8_t oam_addr = 0;
	uint8_t scl_oam[0x20];

	uint8_t scl_occupancy;
	uint8_t bg_col;
	uint16_t spr_pattern[0x08][2];
	uint8_t oam_index = 0;
	uint8_t fine_x = 0;
	uint16_t attr_address = 0;
	uint8_t read_buffer = 0;

	uint8_t ppu_flags = 0;
	uint8_t ppu_mask = 0;
	uint8_t ppu_status = 0;

	sf::Image screen;


	sf::Texture tex;

	Bus* bus = NULL;
	Bus* ppu_bus = NULL;
	uint16_t v = 0x2000;
	uint16_t t = 0x2000;

	uint8_t pattern_address = 0;

	bool addr_latch = false;
	TileInfo current_tile;
	TileInfo cn_tile;
	TileInfo next_tile;
	
	bool even;
public:
	
	uint8_t palettes[0x20];
	int px = 0;
	int line = 0;
	PPU(Bus* _bus, Bus* _ppu_bus)
	{
		tex.create(256, 240);
		screen.create(256, 240);
		bus = _bus;
		ppu_bus = _ppu_bus;
		bus->add_map(this, 0x2000, 0x2000);
	}

	void step();
	void draw(sf::RenderWindow& window);
	uint8_t read(uint16_t address) override;
	uint8_t reverse(uint8_t b)
	{
		//flip tile horrizontally
		b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
		b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
		b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
		return b;
	}
	void write(uint16_t address, uint8_t val) override;
};

