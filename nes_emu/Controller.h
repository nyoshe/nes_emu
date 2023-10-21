#pragma once
#include <iostream>

#include <SFML/Graphics.hpp>
#include "Bus.h"
#include <bitset>
class Controller : public Memory
{
public:
	Controller(Bus* _bus)
	{
		bus = _bus;
		bus->add_map(this, 0x4016, 2);
	}
	uint8_t read(uint16_t address) override
	{
		/*
		if (last_strobe && address == 0x00) {
			control_latch = 0;
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Z); //A
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::X) << 1; //B
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::C) << 2; //select
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) << 3; //start
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Up) << 4; //start
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Down) << 5; //start
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Left) << 6; //start
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Right) << 7; //start

		}
		*/
		//std::cout << std::bitset<8>(control_latch) << std::endl;
		if (address == 0x00 && (last_strobe & 1)) {
			control_latch = 0;
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Z); //A
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::X) << 1; //B
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::C) << 2; //select
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) << 3; //start
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Up) << 4; //start
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Down) << 5; //start
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Left) << 6; //start
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Right) << 7; //start

		}

		if (address == 0x00) {

			uint8_t out = control_latch & 0x01;
			control_latch >>= 1;
			control_latch |= 0x80;
			return out;
		}
		if (address == 0x01) {
			return 0;
		}
		return 1;
	};
	void write(uint16_t address, uint8_t val) override
	{

		if (address == 0x00 && (last_strobe & 1)) {
			control_latch = 0;
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Z); //A
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::X) << 1; //B
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::C) << 2; //select
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) << 3; //start
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Up) << 4; //start
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Down) << 5; //start
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Left) << 6; //start
			control_latch |= sf::Keyboard::isKeyPressed(sf::Keyboard::Right) << 7; //start
			
		}
		if (address == 0x00) {
			last_strobe = val & 1;
		}
		
	};
		

private:
	Bus* bus;
	uint8_t control_latch = 0xFF;
	uint8_t last_strobe = 0;
};

