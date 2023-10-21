#include "PPU.h"

#include <iostream>

void PPU::step() {

	if (px == 341) {
		px = 0;
		line++;
	}
	if (line == 262) line = 0;

	if (px == 0 && line == 0) {
		even ^= 1;
		if (even) {
			px++;
			return;
		}
	}

	//turn on vblank
	if (px == 1 && line == 241) {
		ppu_status |= VBLANK;
		if (ppu_flags & DO_NMI) {
			bus->NMI = true;
		}
	}

	//clear vblank
	if (px == 1 && line == 261) {
		ppu_status &= ~VBLANK;
		ppu_status &= ~SPR0_HIT;
	}

	if (ppu_mask & SHOW_BG) {
		if (px == 256 && (line < 240 || line == 261)) {
			if ((v & 0x7000) != 0x7000) {        // if fine Y < 7}
				v += 0x1000;                     // increment fine Y
			}
			else {
				v &= ~0x7000;                    // fine Y = 0
				uint16_t y = (v & 0x03E0) >> 5;        // let y = coarse Y
				if (y == 29) {
					y = 0;                          // coarse Y = 0
					v ^= 0x0800;                    // switch vertical nametable
				}
				else if (y == 31) {
					y = 0;                          // coarse Y = 0, nametable not switched
				}
					
				else {
					y += 1;                         // increment coarse Y
				}
				v = (v & ~0x03E0) | (y << 5);     // put coarse Y back into v
			}
				
			//vertical increment
			//horizontal increment
		}
		//https://www.nesdev.org/wiki/PPU_scrolling

		//set horizontal v = horizontal t
		if (px == 257 && (line < 240 || line == 261)) {
			oam_addr = 0;
			//v: ....A.. ...BCDEF <- t: ....A.. ...BCDEF
			v = (v & ~0x041F) | (t & 0x041F);
		}

		//set vertical v = vertical t
		//v: GHIA.BC DEF..... <- t: GHIA.BC DEF.....
		if (px >= 280 && px <= 304 && line == 261) {
			v = (v & ~0x7BE0) | (t & 0x7BE0);
		}

		if (px >= 1 && px <= 256 && line < 240){
			int px_offset = 15 - fine_x - (px - 1) % 8;
			int px1 = ((cn_tile.bitplane1 | (current_tile.bitplane1 << 8)) >> (px_offset)) & 1;
			int px2 = ((cn_tile.bitplane2 | (current_tile.bitplane2 << 8)) >> (px_offset)) & 1;
			uint8_t col = (px1 | (px2 << 1));
			bg_col = col;
			uint8_t f_col = 0;
			if(col) {
				f_col = palettes[(px_offset < 8 ? cn_tile.palette : current_tile.palette) * 4 + col];
			}
			else {
				f_col = palettes[0];
			}

			screen.setPixel(px - 1, line, sf::Color(palette_ref[f_col]));
			

		}
		
		if (((px >= 1 && px <= 260) || (px >= 321 && px <= 336)) && (line < 240 || line == 261)) {
			switch ((px - 1) % 8) {
			case 1:
				//read nametable
				pattern_address = ppu_bus->read(((v & 0x0FFF) | 0x2000));
				next_tile.address = pattern_address;
				break;
			case 3: {
				//fetch attribute table
				attr_address = (0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07));

				if ((v & 2) && (v & 64)) {
					//bottom right
					next_tile.palette = (ppu_bus->read(attr_address) & 0xC0) >> 6;
				}
				else if (v & 2) {
					//top right
					next_tile.palette = (ppu_bus->read(attr_address) & 0x0C) >> 2;
				}
				else if (v & 64) {
					//bottom left
					next_tile.palette = (ppu_bus->read(attr_address) & 0x30) >> 4;
				}
				else {
					//top left
					next_tile.palette = (ppu_bus->read(attr_address) & 0x03);
				}
			}
				

				break;
			case 5:
				next_tile.bitplane1 = ppu_bus->read(pattern_address * 16 + ((v & 0x7000) >> 12) + (ppu_flags & BG_ADDR ? 0x1000 : 0));
				break;
			case 7:
				next_tile.bitplane2 = ppu_bus->read(pattern_address * 16 + ((v & 0x7000) >> 12) + 8 + (ppu_flags & BG_ADDR ? 0x1000 : 0));
				current_tile = cn_tile; //maaaaybe fine idk
				cn_tile = next_tile;
				
				//increment horizontal
				if ((v & 0x001F) == 31) { // if coarse X == 31
					v &= ~0x001F;          // coarse X = 0
					v ^= 0x0400;           // switch horizontal nametable
				}
				else {
					v += 1;                // increment coarse X
				}
					
				break;
			}
		}
	} else {
		bg_col = 0;
	}

	if (ppu_mask & SHOW_SPR) {
		if (px >= 2 && px <= 256 && line < 240 && line != 0 && scl_occupancy) {
			//do sprites
			
			for (int i = 0; i < 8; i++) {
				
				if (spr_pattern[i][0] == 0 && spr_pattern[i][1] == 0) {
					scl_occupancy &= ~(1 << i);
					continue;
				}
				if ((scl_occupancy & (1 << i)) && px > scl_oam[i * 4 + 3] && px <= scl_oam[i * 4 + 3] + 8) {
					uint8_t col = (bool(spr_pattern[i][0] & 0x80) | (bool(spr_pattern[i][1] & 0x80) << 1));
					if (!(((scl_oam[i * 4 + 2] & 0x20) && bg_col)) && col) {
						uint8_t f_col = palettes[0x10 + (scl_oam[i * 4 + 2] & 0x03) * 4 + col];
						//if (!col) f_col = palettes[0];

						screen.setPixel(px - 1, line, sf::Color(palette_ref[f_col]));
					}
					
					//check sprite 0
					if ((ppu_mask & SHOW_BG) &&
						i == 0 &&
						col &&
						bg_col &&
						(oam[0] <= (line - 1) &&
							(oam[0] >= (line - 8))) &&
						!(ppu_status & SPR0_HIT)) {
						ppu_status |= SPR0_HIT;
					}

					spr_pattern[i][0] <<= 1;
					spr_pattern[i][1] <<= 1;

				}
			}
		}
		//check oam
		if (px == 257 && line < 240) {
			//scan through oam

			scl_occupancy = 0;

			int c = 0;
			for (int i = 0; i < 0x100; i += 4) {
				uint8_t line_size = ppu_flags & SPR_SIZE ? 15 : 7;
				if (oam[i] <= line && (oam[i] >= line - line_size)) {
					scl_occupancy |= 1 << c;
					spr_pattern[c][0] = 0;
					spr_pattern[c][1] = 0;
					scl_oam[c * 4] = oam[i]; // y coordinate
					scl_oam[c * 4 + 1] = oam[i + 1]; //tile #
					scl_oam[c * 4 + 2] = oam[i + 2]; //flags
					scl_oam[c * 4 + 3] = oam[i + 3]; //x coordinate

					uint8_t spr_flags = oam[i + 2];

					uint16_t base_address = 0;
					if (ppu_flags & SPR_SIZE) {
						if (spr_flags & 0x80) {
							base_address = (oam[i + 1] & 0xFE) * 16 + 16 * (1 - (line - oam[i]) / 8) + ((oam[i + 1] & 1) ? 0x1000 : 0);
						} else {
							base_address = (oam[i + 1] & 0xFE) * 16 + 16 * ((line - oam[i]) / 8) + ((oam[i + 1] & 1) ? 0x1000 : 0);
						}
						
					} else {
						base_address = oam[i + 1] * 16 + ((ppu_flags & SPR_ADDR) ? 0x1000 : 0);
					}
					
					if (spr_flags & 0x80) {
						//flip tile vertically
						spr_pattern[c][0] = ppu_bus->read(base_address + ((7 - (line - oam[i]) % 8)));
						spr_pattern[c][1] = ppu_bus->read(base_address + 8 + ((7 - (line - oam[i]) % 8)));
					} else {
						spr_pattern[c][0] = ppu_bus->read(base_address + (line - oam[i]) % 8);
						spr_pattern[c][1] = ppu_bus->read(base_address + 8 + (line - oam[i]) % 8);
					}
					if (spr_flags & 0x40) {
						//flip horizontally
						spr_pattern[c][0] = reverse(spr_pattern[c][0]);
						spr_pattern[c][1] = reverse(spr_pattern[c][1]);
					}


					c++;
				}
				if (c == 8) {
					//sprite overflow, TODO: set flags
					break;
				}
			}
		}
	}
	
	px++;

}

void PPU::draw(sf::RenderWindow& window)
{
	//screen.create(256, 240);
	/*
	for (int tile = 0; tile < 960; tile++) {
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				int px1 = (ppu_bus->read(ppu_bus->read(0x2000 + tile) * 0x10 + y) >> x) & 1;
				int px2 = (ppu_bus->read(ppu_bus->read(0x2000 + tile) * 0x10 + 8 + y) >> x) & 1;
				int col = 64 * (px1 | (px2 << 1));
				screen.setPixel((tile % 32) * 8 + 7 - x, (tile / 32) * 8 + y, sf::Color(col, col, col));
			}
		}
	}
	*/
	tex.update(screen);
	sf::Sprite spr(tex);
	spr.setScale(2, 2);
	ImGui::Begin("output");
	ImGui::Image(spr);
	ImGui::End();
}

uint8_t PPU::read(uint16_t address)
{
	uint8_t ret_val = 0;
	switch (address % 8) {

	case PPUSTATUS:
		addr_latch = false;
		ret_val = ppu_status;
		//clear vblank flag
		ppu_status &= ~VBLANK;
		break;
	case OAMDATA:
		ret_val = oam[oam_addr];
		oam_addr++;
		break;
	case PPUDATA:
		ret_val = read_buffer;
		read_buffer = ppu_bus->read(v);
		v += VRAM_INC & ppu_flags ? 32 : 1;
		break;
	default:
		return 0;
	}
	return ret_val;
}

void PPU::write(uint16_t address, uint8_t val)
{
	switch(address % 8) {
	case PPUCTRL:
		ppu_flags = val & 0xFC;
		t = (t & ~0x0C00) | ((val & 0x03) << 10);
		break;
	case PPUMASK:
		ppu_mask = val;
		break;
	case OAMADDR:
		oam_addr = val;
		break;
	case OAMDATA:
		oam[oam_addr] = val;
		oam_addr++;
		break;
	case PPUSCROLL:
		if (addr_latch) {
			//t: FGH..AB CDE..... <- d: ABCDEFGH
			//copy ABCDE
			t = (t & ~(0xF8 << 2)) | ((val & 0xF8) << 2);
			//copy FGH
			t = (t & ~(0x7000)) | ((val & 0x07) << 12);
		}
		else {
			//t: ....... ...ABCDE <- d: ABCDE...
			t = (t & ~0x001F) | (val >> 3);
			fine_x = val & 0x7;
		}

		addr_latch ^= 1;
		//v = t;
		break;
	case PPUADDR:
		if (addr_latch) {
			t = (t & 0x7F00) | val;
			v = t;
		} else {
			t = (t & 0x40FF) | ((val & 0x3F) << 8);
			t &= 0x3FFF;
		}

		addr_latch ^= 1;
		break;
	case PPUDATA:
		if ((v & 0x3FFF) >= 0x3F00 ) {
			uint16_t addr = v % 0x20;
			if (addr == 0 || addr == 0x10) {
				palettes[0x00] = val;
				palettes[0x10] = val;
			} else {
				palettes[addr] = val;
			}
			
			v += VRAM_INC & ppu_flags ? 32 : 1;
			break;
		}
		ppu_bus->write(v, val);
		v += VRAM_INC & ppu_flags ? 32 : 1;
		break;
	default:
		break;
	}
}

