#pragma once
#include <cstdint>

enum ALUOP
{
	OR,
	AND,
	EOR,
	ADC,
	SBC,
	ASL,
	LSR,
	ROL,
	ROR,
	PASS,
	NONE, //flags not affected
	CLC,
	SEC,
	CLI,
	SEI,
	CLV,
	SED,
	CLD,
	BIT,
	CMP
};

const uint8_t CARRY = 1;
const uint8_t ZERO = 1 << 1;
const uint8_t INTD = 1 << 2;
const uint8_t DECIMAL = 1 << 3;
const uint8_t BFLAG = 1 << 4;
const uint8_t UNUSEDFLAG = 1 << 5;
const uint8_t OVF = 1 << 6;
const uint8_t NEG = 1 << 7;

class ALU
{
private:

public:
	uint16_t out = 0;
	uint8_t* dest = NULL; //return value
	const uint8_t* bi = NULL; //return value
	const uint8_t* ai = NULL; //return value
	uint8_t* flags = NULL; //return value
	bool carry_disable = false;
	ALUOP aluop = ALUOP::PASS;
	void set_carry()
	{
		//set carry
		*flags = carry_disable ? *flags : (*flags & ~CARRY) | ((out & 0x100) >> 8);
	}
	void set(uint8_t* _dest, const uint8_t* _ai, const uint8_t* _bi, ALUOP _op)
	{
		dest = _dest;
		bi = _bi;
		ai = _ai;
		aluop = _op;
	}
	void op(uint8_t* _flags)
	{
		flags = _flags;
		out = 0;
		switch(aluop) {
		case ALUOP::OR: out = *ai | *bi; break;
		case ALUOP::AND: out = *ai & *bi; break;
		case ALUOP::EOR: out = *ai ^ *bi; break;
		case ALUOP::ADC:out = *ai + *bi + (*flags & 1 & !carry_disable); set_carry();
			*flags = carry_disable ? *flags : (*flags & ~OVF) | (((*bi ^ (out & 0xFF)) & (*ai ^ (out & 0xFF)) & 0x80) >> 1);
			break;
		case ALUOP::CMP: out = *ai - *bi;

			//set zero
			*flags = (*flags & ~ZERO) | (!(out & 0xFF) << 1);
			//set negative
			*flags = (*flags & ~NEG) | (out & 0x80);
			//carry
			*flags = (*flags & ~CARRY) | !((out & 0x100) >> 8); return;

		case ALUOP::SBC: out = *ai - *bi - (1 - (*flags & 1)) * !carry_disable;
			*flags = carry_disable ? *flags : (*flags & ~CARRY) | ((~out & 0x100) >> 8);
			*flags = carry_disable ? *flags : (*flags & ~OVF) | (((uint8_t(255 - *bi) ^ (out & 0xFF)) & (*ai ^ (out & 0xFF)) & 0x80) >> 1);
			break;
		case ALUOP::ASL: out = *bi << 1; set_carry(); break;
		case ALUOP::ROL: out = *bi << 1; out |= *flags & 1; set_carry(); break;
		case ALUOP::LSR: out = *bi >> 1; out |= (*bi & 1) << 8; set_carry(); break;
		case ALUOP::ROR:
			out = *bi >> 1;
			out |= (*flags & 1) << 7;
			out |= (*bi & 1) << 8;
			set_carry();
			break;
		case ALUOP::PASS: out = *bi; break;
		case ALUOP::NONE: if (dest) *dest = static_cast<uint8_t>(*bi); return;
		case ALUOP::SEC: *flags |= CARRY; return;
		case ALUOP::CLC: *flags &= ~CARRY; return;
		case ALUOP::SEI: *flags |= INTD; return;
		case ALUOP::CLI: *flags &= ~INTD; return;
		case ALUOP::CLV: *flags &= ~OVF; return;
		case ALUOP::SED: *flags |= DECIMAL; return;
		case ALUOP::CLD: *flags &= ~DECIMAL; return;
		case ALUOP::BIT:
			//clear and set top bits
			*flags = (*flags & ~0xC0) | (0xC0 & *bi);
			//set zero if and is zero
			*flags = (*flags & ~ZERO) | (((*bi & *ai) == 0) << 1);
			return;
		}
		carry_disable = false;

		//set zero
		*flags = (*flags & ~ZERO) | (!(out & 0xFF) << 1);

		//set negative
		*flags = (*flags & ~NEG) | (out & 0x80);

		out = static_cast<uint8_t>(out);
		if (dest) {
			*dest = static_cast<uint8_t>(out);
		}

	}
};

