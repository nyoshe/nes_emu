#pragma once
#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include "Bus.h"
#include "PPU.h"
#include "imgui-sfml/imgui-SFML.h"
#include "imgui/imgui.h"
#include "Mapper.h"
#include "Ram.h"
const extern uint32_t palette_ref[64];

class WRam : Memory
{
    uint8_t read(uint16_t address) override
    {
        return ram[address];
    };
    uint8_t peek(uint16_t address) override
    {
        return ram[address];
    };
    void write(uint16_t address, uint8_t val) override
    {
        ram[address] = val;
        //std::cout << "can't write to character rom" << std::endl;
    };
    uint8_t ram[0x2000];
};

//sort of just creates the mapper
class Cart :
    public Memory
{
public:
    Cart(Bus* _bus, Bus* _ppu_bus)
    {
        ppu_bus = _ppu_bus;
        bus = _bus;
        tex.create(256, 128);
        image.create(256, 128);
    }
    ~Cart()
    {
        delete mapper;
    }

    void load(std::string file)
    {
        std::ifstream rom_file(file, std::ios::in | std::ios::binary);
        std::vector<uint8_t> rom_arr;
        while (rom_file) {
            rom_arr.push_back(rom_file.get());
        }

        map_num = rom_arr[6] >> 4;
        map_num |= rom_arr[7] & 0xF0;


        switch (map_num) {
        case 0: mapper = new MMC0(); break;
        case 1: mapper = new MMC1(); break;
        case 2: mapper = new INES2(); break;
        case 3: mapper = new INES3(); break;
        default:
            std::cout << "unknown mapper\n";
            break;
        }

        bus->add_map(mapper, 0x8000, 0x8000);
        ppu_bus->add_map(&mapper->chr_rom, 0x00, 0x2000);
        ppu_bus->add_map(&mapper->ppu_mem, 0x2000, 0x1F00);
        //mirror address
        ppu_bus->add_map(&mapper->ppu_mem, 0x3000, 0x0F00);

        /*
        //byte 6
        76543210
        ||||||||
        |||||||+- Mirroring: 0: horizontal (vertical arrangement) (CIRAM A10 = PPU A11)
        |||||||              1: vertical (horizontal arrangement) (CIRAM A10 = PPU A10)
        ||||||+-- 1: Cartridge contains battery-backed PRG RAM ($6000-7FFF) or other persistent memory
        |||||+--- 1: 512-byte trainer at $7000-$71FF (stored before PRG data)
        ||||+---- 1: Ignore mirroring control or above mirroring bit; instead provide four-screen VRAM
        ++++----- Lower nybble of mapper number
        */
        if (rom_arr[6] & 2) {
            bus->add_map((Memory*)&w_ram, 0x6000, 0x2000);
        }
        if (rom_arr[6] & 1) {
            mapper->ppu_mem.mirroring = VERTICAL;
        } else {
            mapper->ppu_mem.mirroring = HORIZONTAL;
        }


        /*
        //byte 7
        76543210
        ||||||||
        |||||||+- VS Unisystem
        ||||||+-- PlayChoice-10 (8 KB of Hint Screen data stored after CHR data)
        ||||++--- If equal to 2, flags 8-15 are in NES 2.0 format
        ++++----- Upper nybble of mapper number
        */


        

        mapper->prg_rom_size = rom_arr[4] * 0x4000;
        mapper->chr_rom_size = rom_arr[5] * 0x2000;
        if (mapper->chr_rom_size == 0) {
            mapper->chr_rom.chr_ram = true;
            mapper->full_chr_rom = new uint8_t[0x2000];
        }
        else {
            mapper->full_chr_rom = new uint8_t[mapper->chr_rom_size];
        }
        mapper->full_rom = new uint8_t[mapper->prg_rom_size];


        rom_arr.erase(rom_arr.begin(), rom_arr.begin() + 16);


	    for (int i = 0; i < mapper->prg_rom_size; i++) {
            mapper->full_rom[i] = rom_arr[i];
	    }

        for (int i = 0; i < mapper->chr_rom_size; i++) {
            mapper->full_chr_rom[i] = rom_arr[i + mapper->prg_rom_size];
        }
        mapper->chr_rom.rom_1 = mapper->full_chr_rom;
        mapper->chr_rom.rom_2 = (mapper->full_chr_rom + sizeof(uint8_t) * 0x1000);
        //std::cout << "done reading" << std::endl;
        rom_file.close();
    }

    void draw_chr_rom(sf::RenderTarget& window, PPU* ppu)
    {
        for(int tile = 0; tile < 256; tile++) {
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int px1 = (mapper->chr_rom.rom_1[tile * 0x10 + y] >> x) & 1;
                    int px2 = (mapper->chr_rom.rom_1[tile * 0x10 + 8 + y] >> x) & 1;
                    uint8_t col = (px1 | (px2 << 1));
                    uint8_t f_col = ppu->palettes[col];
                    image.setPixel((tile % 16) * 8 + 7 - x, (tile / 16) * 8 + y, sf::Color(palette_ref[f_col]));
                }
            }
        }

        for (int tile = 256; tile < 512; tile++) {
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int px1 = (mapper->chr_rom.rom_2[tile * 0x10 + y - 0x1000] >> x) & 1;
                    int px2 = (mapper->chr_rom.rom_2[tile * 0x10 + 8 + y - 0x1000] >> x) & 1;
                    uint8_t col = (px1 | (px2 << 1));
                    uint8_t f_col = ppu->palettes[col];
                    uint32_t aaaa = (sf::Color(0x4, 0x2, 0x5)).toInteger();
                    image.setPixel(128 + (tile % 16) * 8 + 7 - x, ((tile - 256) / 16) * 8 + y, sf::Color(palette_ref[f_col]));
                }
            }
        }


        tex.update(image);
        sf::Sprite spr(tex);
        spr.setScale(2, 2);
        ImGui::Begin("chr_rom");
        ImGui::Image(spr);
        ImGui::End();

    }
private:
    Bus* ppu_bus;
    Bus* bus;
    uint8_t map_num = 0;
    Mapper* mapper = NULL;
    WRam w_ram;
    sf::Texture tex;
    sf::Image image;
};

