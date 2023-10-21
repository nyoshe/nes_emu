#include "CPU.h"

#include <iomanip>


CPU::CPU(Bus* _bus)
{
	current_inst = &CPU::fetch_opcode;
	bus = _bus;
	PCL = read(0xFFFC);
	PCH = read(0xFFFD);
}

void CPU::reset(Bus* _bus)
{
	current_inst = &CPU::fetch_opcode;
	bus = _bus;
	PCL = read(0xFFFC);
	PCH = read(0xFFFD);
	a = 0;
	b = 0;
	x = 0;
	y = 0;
	sp = 0xFD;
	flags = 0x24;
	db = 0;
	rw = 1;
	cycle_count = 6;
	cycle = 0;

	amode = AMODE::IMPL;
	wmode = WMODE::READ;
}

void CPU::step()
{

	cycle++;
	cycle_count++;
	(this->*current_inst)();
}

void CPU::NOI()
{
	std::cout << "instruction not implemented!\n";
	transit(&CPU::fetch_opcode);
}

void CPU::write_debug(std::ostream& s_out) {
	char output[128];
	std::stringstream ss;
	sprintf_s(output, 128, "%4X", pc_debug);

	ss << output;

	sprintf_s(output, 128, "%s", get_instr_str().c_str());
	
	ss << std::setw(28) << std::setfill(' ') << std::left << output;

	sprintf_s(output, 128, " A:%02X X:%02X Y:%02X P:%02X SP:%02X", a, x, y, flags, sp);
	ss << output;
	sprintf_s(output, 128, " ECODE:%02X%02X", bus->peek(0x01), bus->peek(0x00));
	ss << output;
	s_out << ss.str();
	
}

std::string CPU::get_instr_str()
{
	char output[128];
	std::stringstream ss;
	std::string mem_fmt;
	std::string addr_fmt;
	switch (amode) {
	case AMODE::IMPL:
	case AMODE::ACC:
		mem_fmt = "  %02X        ";
		break;
	case AMODE::ABS:
	case AMODE::ABSX:
	case AMODE::ABSY:
	case AMODE::IND:
	case AMODE::INDEX_IND:
	case AMODE::IND_INDEX:
		mem_fmt = "  %02X %02X %02X  ";
		break;
	case AMODE::ZPX:
	case AMODE::ZPY:
	case AMODE::IMM:
	case AMODE::ZP:
	case AMODE::REL:
		mem_fmt = "  %02X %02X     ";
		break;
	case AMODE::INVALID:
		mem_fmt = "U!%02X %02X %02X  ";
	}

	switch (amode) {
	case AMODE::ABS: addr_fmt = " $%04X"; break;
	case AMODE::ABSX: addr_fmt = " $%04X,X"; break;
	case AMODE::ABSY: addr_fmt = " $%04X,Y"; break;
	case AMODE::IND: addr_fmt = " ($%04X)"; break;
	case AMODE::INDEX_IND: addr_fmt = " ($%02hhX,X)"; break;
	case AMODE::IND_INDEX: addr_fmt = " ($%02hhX),Y"; break;
	case AMODE::ZPX: addr_fmt = " $%02hhX,X"; break;
	case AMODE::ZPY: addr_fmt = " $%02hhX,Y"; break;
	case AMODE::IMM: addr_fmt = " #$%02hhX"; break;
	case AMODE::ZP: addr_fmt = " $%02hhX"; break;
	case AMODE::REL: addr_fmt = ""; break;

	case AMODE::INVALID:
		addr_fmt = "  UNKNOWN MODE   ";
	}

	sprintf_s(output, 128, mem_fmt.c_str(), next_mem[0], next_mem[1], next_mem[2]);
	ss << output;
	ss << inst_strings[next_mem[0]];


	if (amode == AMODE::REL) {
		uint16_t new_addr = pc_debug + static_cast<int8_t>(next_mem[1]) + 2;
		sprintf_s(output, 128, " $%04X", new_addr);
	}
	else {
		sprintf_s(output, 128, addr_fmt.c_str(), (next_mem[2] << 8) | next_mem[1]);
	}
	ss << output;
	return ss.str();
}

void CPU::fetch_opcode()
{

	fetched = true;
	opcode = read(PC());
	pc_debug = PC();
	//check interrupts
	if (bus->IRQ && !(flags & INTD)) {
		transit(&CPU::IRQ); 
		return;
	}
	if (bus->NMI) {
		transit(&CPU::NMI);
		return;
	}


	//split memory opcode into aaabbbcc
	op_type = (opcode & 0xE0) >> 5;
	address_mode = (opcode & 0x1C) >> 2;
	group = opcode & 0x03;
	amode = AMODE::IMPL;

	switch(opcode) {
		//flags
	case 0x38: SEC(); transit(&CPU::setup_addr); break;
	case 0x18: CLC(); transit(&CPU::setup_addr); break;
	case 0x58: CLI(); transit(&CPU::setup_addr); break;
	case 0x78: SEI(); transit(&CPU::setup_addr); break;
	case 0xB8: CLV(); transit(&CPU::setup_addr); break;
	case 0xF8: SED(); transit(&CPU::setup_addr); break;
	case 0xD8: CLD(); transit(&CPU::setup_addr); break;
		//transfers
	case 0x98: TYA(); transit(&CPU::setup_addr); break;
	case 0xA8: TAY(); transit(&CPU::setup_addr); break;
	case 0x8A: TXA(); transit(&CPU::setup_addr); break;
	case 0x9A: TXS(); transit(&CPU::setup_addr); break;
	case 0xAA: TAX(); transit(&CPU::setup_addr); break;
	case 0xBA: TSX(); transit(&CPU::setup_addr); break;
		//increment/decrement
	case 0xCA: DEX(); transit(&CPU::setup_addr); break;
	case 0xE8: INX(); transit(&CPU::setup_addr); break;
	case 0x88: DEY(); transit(&CPU::setup_addr); break;
	case 0xC8: INY(); transit(&CPU::setup_addr); break;
		//stack
	case 0x08: transit(&CPU::PHP); break;
	case 0x28: transit(&CPU::PLP); break;
	case 0x48: transit(&CPU::PHA); break;
	case 0x68: transit(&CPU::PLA); break;
		//other
	case 0xEA: NOP(); transit(&CPU::setup_addr); break;
		//branches
	case 0x10: transit(&CPU::BPL); amode = AMODE::REL; break;
	case 0x30: transit(&CPU::BMI); amode = AMODE::REL; break;
	case 0x50: transit(&CPU::BVC); amode = AMODE::REL; break;
	case 0x70: transit(&CPU::BVS); amode = AMODE::REL; break;
	case 0x90: transit(&CPU::BCC); amode = AMODE::REL; break;
	case 0xB0: transit(&CPU::BCS); amode = AMODE::REL; break;
	case 0xD0: transit(&CPU::BNE); amode = AMODE::REL; break;
	case 0xF0: transit(&CPU::BEQ); amode = AMODE::REL; break;
	case 0x00: transit(&CPU::BRK); break;
	case 0x20: transit(&CPU::JSR); amode = AMODE::ABS;  break;
	case 0x40: transit(&CPU::RTI); break;
	case 0x60: transit(&CPU::RTS); break;
	case 0x4C: JMA(); transit(&CPU::setup_addr); break;
	case 0x6C: JMP(); transit(&CPU::setup_addr); break;

		//invalid nops
	case 0x80: NOP(); transit(&CPU::setup_addr);  amode = AMODE::IMM; break;
	default:
		amode = gmodes[group][address_mode];

		//special cases
		amode = opcode == 0x96 ? ZPY : amode;
		amode = opcode == 0xBE ? ABSY : amode;
		amode = opcode == 0xB6 ? ZPY : amode;
		//if we should use group addressing
		switch (opcode) {
			//group 2 nops
			case 0x1A: case 0x3A: case 0x5A: case 0x7A: case 0xDA: case 0xFA:
			NOP(); break;
			//group 0 nops
			case 0x1C: case 0x3C: case 0x5C: case 0x7C: case 0xDC: case 0xFC:
			case 0x14: case 0x34: case 0x54: case 0x74: case 0xD4: case 0xF4:
			NOP(); break;
			default: (this->*group_arr[group][op_type])(); break;
		}


		transit(&CPU::setup_addr);
		break;
	}

	inc_pc();
}

void CPU::inc_pc()
{
	PCH += PCL == 0xFF;
	PCL++;
}

void CPU::setup_addr()
{
	switch (amode) {
	case AMODE::IMPL:
		switch (cycle) {
		case 1: read(PC()); break;
		case 2: alu.op(&flags); fetch_opcode(); break;
		default: return;
		}
		break;
	case AMODE::ACC:
		switch (cycle) {
		case 1: alu.dest = &a; alu.bi = &a; read(PC()); break;
		case 2: alu.op(&flags); fetch_opcode(); break;
		default: return;
		}
		break;
	case AMODE::IMM:
		switch (cycle) {
		case 1: b = read(PC()); inc_pc(); break;
		case 2: alu.op(&flags); fetch_opcode(); break;
		default: return;
		}
		break;
	case AMODE::ZP:
		switch (cycle) {
		case 1: ADH = 0; ADL = read(PC()); inc_pc(); transit(&CPU::writeback); break;
		default: return;
		}
		break;
	case AMODE::ABS:
		switch (cycle) {
		case 1: ADH = 0; ADL = read(PC()); inc_pc(); break;
		case 2: ADH = read(PC()); inc_pc(); transit(&CPU::writeback); break;
		default: return;
		}
		break;
	case AMODE::ABSX:
		switch (cycle) {
		case 1: ADH = 0; ADL = read(PC()); inc_pc(); break;
		case 2:
			ADH = read(PC());
			overflow = uint16_t(ADL + x) > 0xFF;
			ADL += x;
			inc_pc();
			break;
		case 3:
			if (overflow) {
				read(AD());
				ADH++;
				transit(&CPU::writeback);
			}
			else if (wmode == READ) transit_now(&CPU::writeback);
			else read(AD()), transit(&CPU::writeback);
			break;
		default: return;
		}
		break;
	case AMODE::ABSY:
		switch (cycle) {
		case 1: ADH = 0; ADL = read(PC()); inc_pc(); break;
		case 2:
			ADH = read(PC());
			overflow = uint16_t(ADL + y) > 0xFF;
			ADL += y;
			inc_pc();
			break;
		case 3:
			if (overflow) {
				read(AD());
				ADH++;
				transit(&CPU::writeback);
			}
			else if (wmode == READ) transit_now(&CPU::writeback);
			else read(AD()), transit(&CPU::writeback);
			break;
		default: return;
		}
		break;
	case AMODE::ZPX:
		switch (cycle) {
		case 1: ADH = 0; ADL = read(PC()); inc_pc(); break;
		case 2: read(AD()); ADL+=x; transit(&CPU::writeback);  break;
		default: return;
		}
		break;
	case AMODE::ZPY:
		switch (cycle) {
		case 1: ADH = 0; ADL = read(PC()); inc_pc(); break;
		case 2: read(AD()); ADL+=y; transit(&CPU::writeback);  break;
		default: return;
		}
		break;
	case AMODE::INDEX_IND:
		switch (cycle) {
		case 1: ADH = 0; ADL = read(PC()); inc_pc(); break;
		case 2: read(AD()); ADL += x;  break;
		case 3: b = read(AD()); ADL++; break;
		case 4: ADH = read(AD()); ADL = b; transit(&CPU::writeback); break;
		default: return;
		}
		break;
	case AMODE::IND_INDEX:
		switch (cycle) {
		case 1: ADH = 0; ADL = read(PC()); inc_pc(); break;
		case 2: b = read(AD()); ADL++;  break;
		case 3: ADH = read(AD()); overflow = uint16_t(y + b) > 0xFF; ADL = int8_t(b) + y; break;
		case 4: 
			if (overflow) {
				read(AD());
				ADH++;
				transit(&CPU::writeback);
			}
			else if (wmode == READ) transit_now(&CPU::writeback);
			else read(AD()), transit(&CPU::writeback);
			break;
		default: return;
		}
		break;
	default:
		NOP();
		transit(&CPU::fetch_opcode);
	}


}

void CPU::writeback()
{
	switch (wmode) {
	case WMODE::READ:
		switch (cycle) {
		case 1: b = read(AD()); break;
		case 2: alu.op(&flags); fetch_opcode(); break;
		default: return;
		}
		break;
	case WMODE::RMW:
		switch (cycle) {
		case 1: b = read(AD()); break;
		case 2: write(AD(), b); alu.op(&flags); break;
		case 3: transit(&CPU::fetch_opcode); write(AD(), db); break;
		default: return;
		}
		break;
	case WMODE::WRITE:
		switch (cycle) {
		case 1: transit(&CPU::fetch_opcode); write(AD(), *alu.bi); break;
		default: return;
		}
		break;
	case WMODE::JMPABS:
		switch (cycle) {
		case 1: PCL = ADL; PCH = ADH; fetch_opcode(); break;
		default: return;
		}
		break;
	case WMODE::JMP:
		switch (cycle) {
		case 1: PCL = read(AD()); break;
		case 2: transit(&CPU::fetch_opcode); PCH = read(ADH, ADL + 1); break;
		default: return;
		}
		break;
	}
}

//special instructions
void CPU::B_TAKEN() {

	switch (cycle) {
	case 1: b = read(PC()); inc_pc(); break;
	case 2: read(PC()); overflow = uint16_t(PCL + int8_t(b)) > 0xFF; PCL += int8_t(b); break;
	case 3:
		if (overflow) {
			read(PC()); //dummy read
			if (int8_t(b) < 0) {
				PCH--;
			} else {
				PCH++;
			}
			
		}
		else { fetch_opcode(); }
		break;
	case 4: fetch_opcode();
	}
}
void CPU::B_FAIL() {
	switch (cycle) {
	case 1: b = read(PC()); inc_pc(); transit(&CPU::fetch_opcode); break;
	}
}

void CPU::OAMDMA() {
	static uint8_t offset = 0;
	static uint8_t ram_data = 0;
	//not accounting for 513/514 cycles, only take 514 cycles
	if (cycle == 1) {
		offset = 0;
	}
	else if (cycle >= 2 && cycle <= 513){
		if (cycle % 2) {
			bus->write(0x2004, ram_data);
		} else {
			ram_data = bus->read((oam_start << 8) + offset);
			offset++;
		}
			
	} else {
		transit(&CPU::fetch_opcode);
	}
}

void CPU::BRK()
{
	switch (cycle) {
		case 1: read(PC()); inc_pc(); break;
		case 2: write(0x01, sp--, PCH); break;
		case 3: write(0x01, sp--, PCL); break;
		case 4: write(0x01, sp--, flags | BFLAG); flags |= INTD; break;
		case 5: PCL = read(0xFFFE); break;
		case 6: PCH = read(0xFFFF); transit(&CPU::fetch_opcode); break;
	}
}
void CPU::IRQ()
{
	switch (cycle) {
	case 1: read(PC()); bus->IRQ = false; break;
	case 2: write(0x01, sp--, PCH); break;
	case 3: write(0x01, sp--, PCL); break;
	case 4: write(0x01, sp--, flags & ~BFLAG); flags |= INTD; break;
	case 5: PCL = read(0xFFFE); break;
	case 6: PCH = read(0xFFFF); transit(&CPU::fetch_opcode); break;
	}
}

void CPU::NMI()
{
	switch (cycle) {
	case 1: read(PC()); bus->NMI = false; break;
	case 2: write(0x01, sp--, PCH); break;
	case 3: write(0x01, sp--, PCL); break;
	case 4: write(0x01, sp--, flags & ~BFLAG); flags |= INTD; break;
	case 5: PCL = read(0xFFFA); break;
	case 6: PCH = read(0xFFFB); transit(&CPU::fetch_opcode); break;
	}
}

void CPU::JSR() {
	switch(cycle) {
	case 1: ADH = 1; ADL = read(PC()); inc_pc(); break;
	case 2: /*???*/ ; break;
	case 3: write(0x01, sp--, PCH); break;
	case 4: write(0x01, sp--, PCL); break;
	case 5: PCH = read(PC()); PCL = ADL; transit(&CPU::fetch_opcode); break;
	}
}

void CPU::RTI()
{
	switch (cycle) {
	case 1: read(PC()); break;
	case 2: sp++; break;
	case 3: flags = read(0x01, sp++); break;
	case 4: PCL = read(0x01, sp++); break;
	case 5: PCH = read(0x01, sp); transit(&CPU::fetch_opcode); break;
	}
}
void CPU::RTS()
{
	switch (cycle) {
	case 1: read(PC()); break;
	case 2: sp++; break;
	case 3: PCL = read(0x01, sp++); break;
	case 4: PCH = read(0x01, sp); break;
	case 5: inc_pc(); transit(&CPU::fetch_opcode); break;
	}
}
void CPU::PHP()
{
	switch (cycle) {
	case 1: read(PC()); break;
	case 2: write(0x01, sp, flags | UNUSEDFLAG | BFLAG); sp--; transit(&CPU::fetch_opcode);  break;
	}
}
void CPU::PLP()
{
	switch (cycle) {
	case 1: read(PC()); break;
	case 2: sp++; break;
	case 3: flags = 0xEF & read(0x01, sp) | UNUSEDFLAG; transit(&CPU::fetch_opcode);  break;
	}
}
void CPU::PHA()
{
	switch (cycle) {
	case 1: read(PC()); break;
	case 2: write(0x01, sp, a); sp--; transit(&CPU::fetch_opcode);  break;
	}
}
void CPU::PLA()
{
	//bit of a hack, needs to set flags
	alu.set(&a, NULL, &a, ALUOP::PASS);
	switch (cycle) {
	case 1: read(PC()); break;
	case 2: sp++; break;
	case 3: a = read(0x01, sp); alu.op(&flags); transit(&CPU::fetch_opcode);  break;
	}
}
