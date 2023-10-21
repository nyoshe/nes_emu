#pragma once
#include <cstdint>
#include <vector>

#include "Bus.h"
#include "Memory.h"

#include "imgui-sfml/imgui-SFML.h"
#include "imgui/imgui.h"
class Ram : public Memory
{
	uint8_t mem[0x0800];
	Bus* bus;
public:
	Ram(Bus* _bus)
	{
		bus = _bus;
		bus->add_map(this, 0x0000, 0x0800);
	}
	uint8_t read(uint16_t address) override;
	uint8_t peek(uint16_t address) override { return mem[address]; };
	void write(uint16_t address, uint8_t val) override;
	void draw()
	{
		ImGui::Begin("ram");
		ImGui::BeginTable("ram watch", 18, ImGuiTableFlags_RowBg);
		ImGui::TableNextColumn();
		ImGui::TableNextColumn();
		for (int j = 0; j < 16; j++) {
			ImGui::TableNextColumn();
			ImGui::Text("x%01X", j);
		}
		ImGui::TableNextRow();

		for (int i = 0; i < 32; i++) {

			ImGui::TableNextColumn();
			ImGui::Text("%02Xx", i);
			ImGui::TableNextColumn();

			for (int j = 0; j < 16; j++) {
				ImGui::TableNextColumn();
				ImGui::Text("%02X", mem[i * 16 + j]);
			}
			ImGui::TableNextRow();
		}
		ImGui::EndTable();
		ImGui::End();
	}
};

