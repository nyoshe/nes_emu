#pragma once
#include "Memory.h"

enum Mirroring
{
    SINGLE_LOW,
    SINGLE_HIGH,
    VERTICAL,
    HORIZONTAL
};

struct PPUMem :
    public Memory
{
    uint16_t gen_address(uint16_t address)
    {
        switch (mirroring) {
        case VERTICAL:
            if (address >= 0x800) {
                return (address - 0x800);
            }
            return (address);
            break;
        case HORIZONTAL:
            if (address >= 0x800) {
                return ((address % 0x400) + 0x800);
            }
            return (address % 0x400);
            break;
        case SINGLE_LOW:
            return address % 0x400;
        case SINGLE_HIGH:
            return address % 0x400 + 0x800;
        }

    }
    uint8_t read(uint16_t address) override
    {
        return vram[gen_address(address)];
    };
    uint8_t peek(uint16_t address) override
    {
        return vram[gen_address(address)];
    };
    void write(uint16_t address, uint8_t val) override
    {
        vram[gen_address(address)] = val;
    };
    uint8_t vram[0x1000];
    Mirroring mirroring;
};

struct CharRom :
    public Memory
{

    uint8_t read(uint16_t address) override
    {
        if (address >= 0x1000) return rom_2[address - 0x1000];
        return rom_1[address];
    };
    uint8_t peek(uint16_t address) override
    {
        if (address >= 0x1000) return rom_2[address - 0x1000];
        return rom_1[address];
    };
    void write(uint16_t address, uint8_t val) override
    {
        //if (address >= 0x1c00) {
        //    std::cout << "test";
        //}
        if (address >= 0x1000) {
            if (chr_ram) {
                rom_2[address - 0x1000] = val;
            }
        }
        else {
            if (chr_ram) {
                rom_1[address] = val;
            }
        }
        //std::cout << "can't write to character rom" << std::endl;
    };
public:
    uint8_t* rom_1 = NULL;
    uint8_t* rom_2 = NULL;
    bool chr_ram = false;
};

class Mapper : public Memory
{
public:

    ~Mapper()
    {
        delete[] full_rom;
        delete[] full_chr_rom;
    }
    virtual uint8_t read(uint16_t address)
    {
        return full_rom[address % prg_rom_size];
    };
    virtual uint8_t peek(uint16_t address)
    {
        return full_rom[address % prg_rom_size];
    };
    virtual void write(uint16_t address, uint8_t val)
    {
        full_rom[address % prg_rom_size] = val;
        //std::cout << "can't write to character rom" << std::endl;
    };
    PPUMem ppu_mem = PPUMem();
    CharRom chr_rom = CharRom();

    uint8_t* full_rom = NULL;
    uint8_t* full_chr_rom = NULL;

    int prg_rom_size = 0;
    int chr_rom_size = 0;
};


class INES2 : public Mapper
{
public:
    void write(uint16_t address, uint8_t val) override
    {
        prg_offset = (0x4000 * (val & 0xF));
        
    };
    uint8_t read(uint16_t address) override
    {
        if (address >= 0x4000) return full_rom[address + prg_rom_size - 0x8000];
        return full_rom[address % 0x4000 + prg_offset];
    };
    uint8_t peek(uint16_t address) override
    {
        if (address >= 0x4000) return full_rom[address + prg_rom_size - 0x8000];
        return full_rom[address % 0x4000 + prg_offset];
    };
    int prg_offset = 0;
};


class INES3 : public Mapper
{
public:
    void write(uint16_t address, uint8_t val) override
    {
        chr_rom.rom_1 = full_chr_rom + 0x2000 * (val & 0x07);
        chr_rom.rom_2 = 0x1000 + full_chr_rom + 0x2000 * (val & 0x07);
    };
};


class MMC0 : public Mapper
{
    uint8_t read(uint16_t address) override
    {
        return full_rom[address % prg_rom_size];
    };
    uint8_t peek(uint16_t address) override
    {
        return full_rom[address % prg_rom_size];
    };
    void write(uint16_t address, uint8_t val) override
    {
        full_rom[address % prg_rom_size] = val;
        //std::cout << "can't write to character rom" << std::endl;
    };
};

class MMC1 : public Mapper
{
public:

    uint8_t read(uint16_t address) override
    {
        if (address >= 0x4000)return full_rom[address - 0x4000 + prg_offset_2];
        return full_rom[address + prg_offset_1];
    }
    uint8_t peek(uint16_t address) override
    {
        if (address >= 0x4000)return full_rom[address - 0x4000 + prg_offset_2];
        return full_rom[address + prg_offset_1];
    }
    void write(uint16_t address, uint8_t val) override
    {

        shift_reg >>= 1;
        shift_count++;
        if (val & 0x01) shift_reg |= 0x10;
        if (val & 0x80) {
	        shift_reg = 0;
        	shift_count = 0;
        	prg_bank_mode = 3;
            prg_offset_2 = prg_rom_size - 0x4000;
        };

        if (shift_count == 5) {
            switch ((address + 0x8000) & 0xE000) {
                //control
            case 0x8000:
                prg_bank_mode = (shift_reg & 0x0C) >> 2;
                if (prg_bank_mode == 3) prg_offset_2 = prg_rom_size - 0x4000;
                if (prg_bank_mode == 2) prg_offset_1 = 0;
                chr_bank_mode = (shift_reg & 0x10) >> 4;
                mirroring = (shift_reg & 0x03);
                ppu_mem.mirroring = (Mirroring)mirroring;
                break;
            case 0xA000:

                if (chr_bank_mode) {
                    chr_rom.rom_1 = full_chr_rom + shift_reg * 0x1000;
                }
                else {
                    chr_rom.rom_1 = full_chr_rom + (shift_reg & 0xFE) * 0x1000;
                    chr_rom.rom_2 = full_chr_rom + (shift_reg & 0xFE) * 0x1000 + 0x1000;
                }
                break;
            case 0xC000:
                if (chr_bank_mode) chr_rom.rom_2 = full_chr_rom + shift_reg * 0x1000;
                break;
            case 0xE000:
                if (prg_bank_mode < 2) {
                    prg_offset_1 = (shift_reg & 0xFE) * 0x8000;
                    prg_offset_2 = (shift_reg & 0xFE) * 0x8000 + 0x4000;
                }
                if (prg_bank_mode == 2) {
                    //prg_offset_1 = (shift_reg) * 0x4000;
                    prg_offset_2 = (shift_reg) * 0x4000;
                }
                if (prg_bank_mode == 3) {
                    //prg_offset_2 = (shift_reg) * 0x4000;
                    prg_offset_1 = (shift_reg) * 0x4000;
                }
                break;
            }
            shift_reg = 0;
            shift_count = 0;
        }

        //std::cout << "can't write to character rom" << std::endl;
    }
    int chr_offset = 0;
    int prg_offset_1 = 0;
    int prg_offset_2 = 0x4000;
    uint8_t shift_reg = 0;
    uint8_t control_reg = 0;
    uint8_t chr_bank_0 = 0;
    uint8_t chr_bank_1 = 0;
    uint8_t prg_bank = 0;
    /*
    0, 1: switch 32 KB at $8000, ignoring low bit of bank number;
|   2: fix first bank at $8000 and switch 16 KB bank at $C000;
|   3: fix last bank at $C000 and switch 16 KB bank at $8000)
    */
    uint8_t prg_bank_mode = 0;
    //(0: switch 8 KB at a time; 1: switch two separate 4 KB banks)
    uint8_t chr_bank_mode = 0;
    uint8_t mirroring = 0;
    int shift_count = 0;
};