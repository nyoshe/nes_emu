#pragma once
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>   

#include <SFML/Graphics.hpp>

#include "ALU.h"
#include "Bus.h"

#include "imgui-sfml/imgui-SFML.h"
#include "imgui/imgui.h"

//TODO: INC AND DC
extern sf::Font global_font;
extern sf::Font global_font_console;
enum AMODE
{
	IMPL,
	ACC,
	IMM,
	ZP,
	ZPX,
	ZPY,
	REL,
	ABS,
	ABSX,
	ABSY,
	IND,
	INDEX_IND,
	IND_INDEX,
	INVALID
};

enum WMODE
{
	READ,
	RMW,
	WRITE,
	JMP,
	JMPABS
};


class CPU
{
public:

	CPU(Bus* _bus);
	void reset(Bus* _bus);
	void step();
	uint64_t cycle_count = 6;
	void write_debug(std::ostream& s_out);
	std::string get_instr_str();


	bool did_fetch() {
		if (fetched) { fetched = false; return true; }
	}
	void draw_state(sf::RenderTarget& window)
	{
		next_mem[0] = bus->peek(pc_debug);
		next_mem[1] = bus->peek(pc_debug + 1);
		next_mem[2] = bus->peek(pc_debug + 2);

		ImGui::Begin("cpu state");
		sf::Text text;

		if (flags & CARRY) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
		ImGui::Text("C"); ImGui::SameLine();
		if (flags & CARRY) ImGui::PopStyleColor();

		if (flags & ZERO) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
		ImGui::Text("Z"); ImGui::SameLine();
		if (flags & ZERO) ImGui::PopStyleColor();

		if (flags & INTD) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
		ImGui::Text("I"); ImGui::SameLine();
		if (flags & INTD) ImGui::PopStyleColor();

		if (flags & DECIMAL) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
		ImGui::Text("D"); ImGui::SameLine();
		if (flags & DECIMAL) ImGui::PopStyleColor();

		if (flags & OVF) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
		ImGui::Text("V"); ImGui::SameLine();
		if (flags & OVF) ImGui::PopStyleColor();

		if (flags & NEG) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
		ImGui::Text("N");
		if (flags & NEG) ImGui::PopStyleColor();

		ImGui::Columns(3, "registers");
		ImGui::Text("A: %02X", a);
		ImGui::NextColumn();
		ImGui::Text("X: %02X", x);
		ImGui::NextColumn();
		ImGui::Text("Y: %02X", y);
		ImGui::Columns();

		ImGui::Columns(3, "addresses");
		ImGui::Text("PC: %04X", PC());
		ImGui::NextColumn();
		ImGui::Text("AD: %04X", AD());
		ImGui::NextColumn();
		ImGui::Text("sp: %02X", sp);
		ImGui::Columns();


		ImGui::End();
		
		/*


		//draw PC
		text.setPosition(64, 64);
		std::stringstream ss;
		ss << std::hex << std::setw(4) << std::uppercase << std::setfill('0') << PC();
		text.setString("PC:" + ss.str());
		tex.draw(text);

		//draw stack pointer
		ss.str("");
		text.setPosition(64 * 3, 64);
		ss << std::hex << std::setw(2) << std::uppercase << std::setfill('0') << int(sp);
		text.setString("SP:" + ss.str());
		tex.draw(text);

		//draw address
		ss.str("");
		text.setPosition(64 * 5, 64);
		ss << std::hex << std::setw(4) << std::uppercase << std::setfill('0') << ((ADH << 8) + ADL);
		text.setString("AD:" + ss.str());
		tex.draw(text);

		//draw instruciton
		text.setPosition(64, 32 * 3);
		text.setString(get_instr_str());
		tex.draw(text);

		text.setCharacterSize(16);
		text.setFont(global_font_console);
		ss.str("");
		ss << "   ";
		for (int i = 0; i < 16; i++) {
			ss << std::hex << std::setw(1) << std::uppercase << " x" << int(i);
		}
		ss << std::endl;
		text.setPosition(16, 5 * 32 - 24);
		

		for (int i = 0; i < 32; i++) {
			ss << std::hex << std::setw(2) << std::uppercase << std::setfill('0') << int(i) << "x ";
			for (int j = 0; j < 16; j++) {
				ss << std::hex << std::setw(2) << std::uppercase << std::setfill('0');
				ss << int(bus->peek(i * 16 + j)) << " ";
			}
			ss << std::endl;
			
		}
		text.setString(ss.str());
		tex.draw(text);

		sf::Sprite spr;
		spr.setTexture(tex.getTexture());
		spr.setPosition(512, 1024);
		//not sure why I have to do this
		spr.setScale(1, -1);
		window.draw(spr);
		*/
	}
private:
	Bus* bus;
	ALU alu = ALU();
	bool fetched;
	bool overflow = false;
	uint8_t a = 0;
	uint8_t b = 0;
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t sp = 0xFD;
	uint8_t flags = 0x24;
	uint8_t db = 0;
	uint8_t rw = 1;
	const uint8_t one = 1;

	void (CPU::* current_inst)(void) = NULL;

	//high and low address registers
	uint8_t ADL = 0;
	uint8_t ADH = 0;

	//high low program counter
	uint8_t PCH = 0xC0;
	uint8_t PCL = 0x00;

	uint8_t opcode = 0;
	uint16_t cycle;

	//op info
	uint8_t op_type = 0;
	uint8_t address_mode = 0;
	uint8_t group = 0;

	uint8_t next_mem[3] = { 0x00, 0x00, 0x00 };
	uint16_t pc_debug;

	uint8_t oam_start = 0;

	AMODE amode = AMODE::IMPL;
	WMODE wmode = WMODE::READ;

	uint16_t AD() { return (static_cast<uint16_t>(ADH) << 8) | ADL; };
	uint16_t PC() { return (static_cast<uint16_t>(PCH) << 8) | PCL; };


	void log(std::string name)
	{
		std::cout << name << " ";
	}

	void inc_cycle();

	void write(uint16_t AD, uint8_t val)
	{
		//catch OAM DMA
		if (AD == 0x4014) {
			oam_start = val;
			transit(&CPU::OAMDMA);
		}
		bus->write(AD, val);
	}
	void write(uint8_t ADH, uint8_t ADL, uint8_t val)
	{
		write((static_cast<uint16_t>(ADH) << 8) | ADL, val);
	}

	uint8_t read(uint8_t ADH, uint8_t ADL)
	{
		return bus->read(ADH, ADL);
	}
	uint8_t read(uint16_t AD)
	{
		return bus->read(AD);
	}

	//for usage by previous functions

	
	//sub instructions
	void fetch_opcode();
	void inc_pc();

	/*
	Register selection for load and store

		bit1 bit0     A  X  Y
		0    0              x
		0    1        x
		1    0           x
		1    1        x  x
	*/
	//zero page and absolute addressing remain the same across instructions
	void setup_addr();
	void writeback();
	void transit(void (CPU::* op)()) { current_inst = op; cycle = 0; };
	void transit_now(void (CPU::* op)()) { current_inst = op; cycle = 1; (this->*current_inst)(); };

	//branch instructions
	void BPL() { if (flags & NEG) transit_now(&CPU::B_FAIL); else  transit_now(&CPU::B_TAKEN);};
	void BMI() { if (flags & NEG) transit_now(&CPU::B_TAKEN); else  transit_now(&CPU::B_FAIL);};
	void BVC() { if (flags & OVF) transit_now(&CPU::B_FAIL); else transit_now(&CPU::B_TAKEN);};
	void BVS() { if (flags & OVF) transit_now(&CPU::B_TAKEN); else  transit_now(&CPU::B_FAIL);};
	void BCC() { if (flags & CARRY) transit_now(&CPU::B_FAIL); else transit_now(&CPU::B_TAKEN);};
	void BCS() { if (flags & CARRY) transit_now(&CPU::B_TAKEN); else  transit_now(&CPU::B_FAIL);};
	void BNE() { if (flags & ZERO) transit_now(&CPU::B_FAIL); else transit_now(&CPU::B_TAKEN);};
	void BEQ() { if (flags & ZERO) transit_now(&CPU::B_TAKEN); else  transit_now(&CPU::B_FAIL);};

	void B_TAKEN();
	void B_FAIL();

	void OAMDMA();
	//misc functions
	void BRK();
	void IRQ();
	void NMI();
	void JSR();
	void RTI();
	void RTS();
	void DEX() { alu.carry_disable = true; alu.set(&x, &x, &one, ALUOP::SBC); };
	void DEY() { alu.carry_disable = true; alu.set(&y, &y, &one, ALUOP::SBC); };
	void INY() { alu.carry_disable = true; alu.set(&y, &y, &one, ALUOP::ADC); };
	void INX() { alu.carry_disable = true; alu.set(&x, &x, &one, ALUOP::ADC); };
	void CLC() { alu.set(NULL, NULL, NULL, ALUOP::CLC); };
	void SEC() { alu.set(NULL, NULL, NULL, ALUOP::SEC); };
	void CLD() { alu.set(NULL, NULL, NULL, ALUOP::CLD); };
	void SED() { alu.set(NULL, NULL, NULL, ALUOP::SED); };
	void CLI() { alu.set(NULL, NULL, NULL, ALUOP::CLI); };
	void SEI() { alu.set(NULL, NULL, NULL, ALUOP::SEI); };
	void CLV() { alu.set(NULL, NULL, NULL, ALUOP::CLV); };
	void TYA() { alu.set(&a, NULL, &y, ALUOP::PASS); };
	void TAY() { alu.set(&y, NULL, &a, ALUOP::PASS); };
	void TXA() { alu.set(&a, NULL, &x, ALUOP::PASS); };
	void TXS() { alu.set(&sp, NULL, &x, ALUOP::NONE); };
	void TAX() { alu.set(&x, NULL, &a, ALUOP::PASS); };
	void TSX() { alu.set(&x, NULL, &sp, ALUOP::PASS); };
	void NOP() { wmode = READ; alu.set(NULL, NULL, NULL, ALUOP::NONE); };

	//stack functions
	void PHP();
	void PLP();
	void PHA();
	void PLA();

	//not implemented instruction warning
	void NOI();

	//group 1 instructions, code 01
	
	void ORA() { wmode = READ; alu.set(&a, &a, &b, ALUOP::OR); };
	void AND() { wmode = READ; alu.set(&a, &a, &b, ALUOP::AND); };
	void EOR() { wmode = READ; alu.set(&a, &a, &b, ALUOP::EOR); };
	void ADC() { wmode = READ; alu.set(&a, &a, &b, ALUOP::ADC); };
	void STA() { wmode = WRITE; alu.set(NULL, NULL, &a, ALUOP::NONE); };
	void LDA() { wmode = READ; alu.set(&a, NULL, &b, ALUOP::PASS); };
	void CMP() { wmode = READ; alu.set(NULL, &a, &b, ALUOP::CMP); };
	void SBC() { wmode = READ; alu.set(&a, &a, &b, ALUOP::SBC); };

	//group 2 instructions, code 10
	void ASL() { wmode = RMW; alu.set(&db, NULL, &b, ALUOP::ASL); };
	void ROL() { wmode = RMW; alu.set(&db, NULL, &b, ALUOP::ROL); };
	void LSR() { wmode = RMW; alu.set(&db, NULL, &b, ALUOP::LSR); };
	void ROR() { wmode = RMW; alu.set(&db, NULL, &b, ALUOP::ROR); };
	void STX() { wmode = WRITE; alu.set(NULL, NULL, &x, ALUOP::NONE); };
	void LDX() { wmode = READ; alu.set(&x, &a, &b, ALUOP::PASS); };
	void DEC() { alu.carry_disable = true; wmode = RMW; alu.set(&db, &b, &one, ALUOP::SBC); };
	void INC() { alu.carry_disable = true; wmode = RMW; alu.set(&db, &b, &one, ALUOP::ADC); };
	//group 3 instructions, code 00
	void BIT() { wmode = READ; alu.set(NULL, &a, &b, ALUOP::BIT); };
	void JMP() { amode = ABS; wmode = WMODE::JMP; };
	void JMA() { amode = ABS; wmode = WMODE::JMPABS;  };
	void STY() { wmode = WRITE; alu.set(NULL, NULL, &y, ALUOP::NONE); };
	void LDY() { wmode = READ; alu.set(&y, NULL, &b, ALUOP::PASS); };
	void CPY() { wmode = READ; alu.set(NULL, &y, &b, ALUOP::CMP); };
	void CPX() { wmode = READ; alu.set(NULL, &x, &b, ALUOP::CMP); };



	void (CPU::* group_arr[4][8])(void) = {
	{&CPU::NOP, &CPU::BIT, &CPU::NOP, &CPU::NOP, &CPU::STY, &CPU::LDY, &CPU::CPY, &CPU::CPX},
	{&CPU::ORA, &CPU::AND, &CPU::EOR, &CPU::ADC, &CPU::STA, &CPU::LDA, &CPU::CMP, &CPU::SBC},
    {&CPU::ASL, &CPU::ROL, &CPU::LSR, &CPU::ROR, &CPU::STX, &CPU::LDX, &CPU::DEC, &CPU::INC},
    {&CPU::NOP, &CPU::NOP, &CPU::NOP, &CPU::NOP, &CPU::NOP, &CPU::NOP, &CPU::NOP, &CPU::NOP},
	};
	AMODE gmodes[4][8] = {
		{ IMM, ZP, INVALID, ABS, INVALID, ZPX, INVALID, ABSX },
		{ INDEX_IND, ZP, IMM, ABS, IND_INDEX, ZPX, ABSY, ABSX },
		{ IMM, ZP, ACC, ABS, IND_INDEX, ZPX, IMPL, ABSX },
		{ INDEX_IND, ZP, IMM, ABS, IND_INDEX, ZPX, ABSY, ABSX },
	};

	// table from https://www.masswerk.at/nowgobang/images/6502-instruction-set-1.svg
	void (CPU::* i_table[256])(void) = {
	//  x0         x1         x2         x3         x4         x5         x6         x7         x8         x9         xA         xB         xC         xD         xE         xF
		&CPU::BRK, &CPU::ORA, &CPU::NOI, &CPU::NOI, &CPU::NOP, &CPU::ORA, &CPU::ASL, &CPU::NOI, &CPU::PHP, &CPU::ORA, &CPU::ASL, &CPU::NOI, &CPU::NOI, &CPU::ORA, &CPU::ASL, &CPU::NOI, //0x
		&CPU::BPL, &CPU::ORA, &CPU::NOI, &CPU::NOI, &CPU::NOP, &CPU::ORA, &CPU::ASL, &CPU::NOI, &CPU::CLC, &CPU::ORA, &CPU::NOP, &CPU::NOI, &CPU::NOP, &CPU::ORA, &CPU::ASL, &CPU::NOI, //1x
		&CPU::JSR, &CPU::AND, &CPU::NOI, &CPU::NOI, &CPU::BIT, &CPU::AND, &CPU::ROL, &CPU::NOI, &CPU::PLP, &CPU::AND, &CPU::ROL, &CPU::NOI, &CPU::BIT, &CPU::AND, &CPU::ROL, &CPU::NOI, //2x
		&CPU::BMI, &CPU::AND, &CPU::NOI, &CPU::NOI, &CPU::NOP, &CPU::AND, &CPU::ROL, &CPU::NOI, &CPU::SEC, &CPU::AND, &CPU::NOP, &CPU::NOI, &CPU::NOP, &CPU::AND, &CPU::ROL, &CPU::NOI, //3x
		&CPU::RTI, &CPU::EOR, &CPU::NOI, &CPU::NOI, &CPU::NOI, &CPU::EOR, &CPU::LSR, &CPU::NOI, &CPU::PHA, &CPU::EOR, &CPU::LSR, &CPU::NOI, &CPU::JMP, &CPU::EOR, &CPU::LSR, &CPU::NOI, //4x
		&CPU::BVC, &CPU::EOR, &CPU::NOI, &CPU::NOI, &CPU::NOP, &CPU::EOR, &CPU::LSR, &CPU::NOI, &CPU::CLI, &CPU::EOR, &CPU::NOP, &CPU::NOI, &CPU::NOP, &CPU::EOR, &CPU::LSR, &CPU::NOI, //5x
		&CPU::RTS, &CPU::ADC, &CPU::NOI, &CPU::NOI, &CPU::NOI, &CPU::ADC, &CPU::ROR, &CPU::NOI, &CPU::PLA, &CPU::ADC, &CPU::ROR, &CPU::NOI, &CPU::JMA, &CPU::ADC, &CPU::ROR, &CPU::NOI, //6x
		&CPU::BVS, &CPU::ADC, &CPU::NOI, &CPU::NOI, &CPU::NOP, &CPU::ADC, &CPU::ROR, &CPU::NOI, &CPU::SEI, &CPU::ADC, &CPU::NOP, &CPU::NOI, &CPU::NOP, &CPU::ADC, &CPU::ROR, &CPU::NOI, //7x
		&CPU::NOP, &CPU::STA, &CPU::NOI, &CPU::NOI, &CPU::STY, &CPU::STA, &CPU::STX, &CPU::NOI, &CPU::DEY, &CPU::STA, &CPU::TXA, &CPU::NOI, &CPU::STY, &CPU::STA, &CPU::STX, &CPU::NOI, //8x
		&CPU::BCC, &CPU::STA, &CPU::NOI, &CPU::NOI, &CPU::STY, &CPU::STA, &CPU::STX, &CPU::NOI, &CPU::TYA, &CPU::STA, &CPU::TXS, &CPU::NOI, &CPU::NOI, &CPU::STA, &CPU::STX, &CPU::NOI, //9x
		&CPU::LDY, &CPU::LDA, &CPU::LDX, &CPU::NOI, &CPU::LDY, &CPU::LDA, &CPU::LDX, &CPU::NOI, &CPU::TAY, &CPU::LDA, &CPU::TAX, &CPU::NOI, &CPU::LDY, &CPU::LDA, &CPU::LDX, &CPU::NOI, //Ax
		&CPU::BCS, &CPU::LDA, &CPU::NOI, &CPU::NOI, &CPU::LDY, &CPU::LDA, &CPU::LDX, &CPU::NOI, &CPU::CLV, &CPU::LDA, &CPU::TSX, &CPU::NOI, &CPU::LDY, &CPU::LDA, &CPU::LDX, &CPU::NOI, //Bx
		&CPU::CPY, &CPU::CMP, &CPU::NOI, &CPU::NOI, &CPU::CPY, &CPU::CMP, &CPU::DEC, &CPU::NOI, &CPU::INY, &CPU::CMP, &CPU::DEX, &CPU::NOI, &CPU::CPY, &CPU::CMP, &CPU::DEC, &CPU::NOI, //Cx
		&CPU::BNE, &CPU::CMP, &CPU::NOI, &CPU::NOI, &CPU::NOP, &CPU::CMP, &CPU::DEC, &CPU::NOI, &CPU::CLD, &CPU::CMP, &CPU::NOP, &CPU::NOI, &CPU::NOP, &CPU::CMP, &CPU::DEC, &CPU::NOI, //Dx
		&CPU::CPX, &CPU::SBC, &CPU::NOI, &CPU::NOI, &CPU::CPX, &CPU::SBC, &CPU::INC, &CPU::NOI, &CPU::INX, &CPU::SBC, &CPU::NOP, &CPU::NOI, &CPU::CPX, &CPU::SBC, &CPU::INC, &CPU::NOI, //Ex
		&CPU::BEQ, &CPU::SBC, &CPU::NOI, &CPU::NOI, &CPU::NOP, &CPU::SBC, &CPU::INC, &CPU::NOI, &CPU::SED, &CPU::SBC, &CPU::NOP, &CPU::NOI, &CPU::NOP, &CPU::SBC, &CPU::INC, &CPU::NOI, //Fx
	};
	std::string inst_strings[256]{
	//  x0     x1     x2     x3     x4     x5     x6     x7     x8     x9     xA     xB     xC     xD     xE     xF                                                                                                               */
		"BRK", "ORA", "NOI", "NOI", "NOP", "ORA", "ASL", "NOI", "PHP", "ORA", "ASL", "NOI", "NOI", "ORA", "ASL", "NOI", //0x
		"BPL", "ORA", "NOI", "NOI", "NOP", "ORA", "ASL", "NOI", "CLC", "ORA", "NOP", "NOI", "NOP", "ORA", "ASL", "NOI", //1x
		"JSR", "AND", "NOI", "NOI", "BIT", "AND", "ROL", "NOI", "PLP", "AND", "ROL", "NOI", "BIT", "AND", "ROL", "NOI", //2x
		"BMI", "AND", "NOI", "NOI", "NOP", "AND", "ROL", "NOI", "SEC", "AND", "NOP", "NOI", "NOP", "AND", "ROL", "NOI", //3x
		"RTI", "EOR", "NOI", "NOI", "NOI", "EOR", "LSR", "NOI", "PHA", "EOR", "LSR", "NOI", "JMP", "EOR", "LSR", "NOI", //4x
		"BVC", "EOR", "NOI", "NOI", "NOP", "EOR", "LSR", "NOI", "CLI", "EOR", "NOP", "NOI", "NOP", "EOR", "LSR", "NOI", //5x
		"RTS", "ADC", "NOI", "NOI", "NOI", "ADC", "ROR", "NOI", "PLA", "ADC", "ROR", "NOI", "JMA", "ADC", "ROR", "NOI", //6x
		"BVS", "ADC", "NOI", "NOI", "NOP", "ADC", "ROR", "NOI", "SEI", "ADC", "NOP", "NOI", "NOP", "ADC", "ROR", "NOI", //7x
		"NOP", "STA", "NOI", "NOI", "STY", "STA", "STX", "NOI", "DEY", "STA", "TXA", "NOI", "STY", "STA", "STX", "NOI", //8x
		"BCC", "STA", "NOI", "NOI", "STY", "STA", "STX", "NOI", "TYA", "STA", "TXS", "NOI", "NOI", "STA", "STX", "NOI", //9x
		"LDY", "LDA", "LDX", "NOI", "LDY", "LDA", "LDX", "NOI", "TAY", "LDA", "TAX", "NOI", "LDY", "LDA", "LDX", "NOI", //Ax
		"BCS", "LDA", "NOI", "NOI", "LDY", "LDA", "LDX", "NOI", "CLV", "LDA", "TSX", "NOI", "LDY", "LDA", "LDX", "NOI", //Bx
		"CPY", "CMP", "NOI", "NOI", "CPY", "CMP", "DEC", "NOI", "INY", "CMP", "DEX", "NOI", "CPY", "CMP", "DEC", "NOI", //Cx
		"BNE", "CMP", "NOI", "NOI", "NOP", "CMP", "DEC", "NOI", "CLD", "CMP", "NOP", "NOI", "NOP", "CMP", "DEC", "NOI", //Dx
		"CPX", "SBC", "NOI", "NOI", "CPX", "SBC", "INC", "NOI", "INX", "SBC", "NOP", "NOI", "CPX", "SBC", "INC", "NOI", //Ex
		"BEQ", "SBC", "NOI", "NOI", "NOP", "SBC", "INC", "NOI", "SED", "SBC", "NOP", "NOI", "NOP", "SBC", "INC", "NOI", //Fx
	};
};

