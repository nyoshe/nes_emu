#pragma once
#include <SFML/Graphics.hpp>

#include "Ram.h"

sf::Font global_font;
sf::Font global_font_console;
#include "Cart.h"
#include "CPU.h"
#include "Controller.h"
#include <iostream>
#include <fstream>
#include <iomanip>

#include "PPU.h"

#include "imgui/imgui.h"
#include "imgui-sfml/imgui-SFML.h"
#include "fileDialog/ImGuiFileDialog.h"

int main()
{
    if (!global_font.loadFromFile("basis33.ttf"))
    {
        std::cout << "failed to load font" << std::endl;
    }
    if (!global_font_console.loadFromFile("CONSOLA.TTF"))
    {
        std::cout << "failed to load font" << std::endl;
    }

    std::ofstream cycle_log("cycle_log.txt", std::ofstream::out);
    std::ofstream log("log.txt", std::ofstream::out);
    std::ofstream cycle_list("cycle_list.txt", std::ofstream::out);
    sf::RenderWindow window(sf::VideoMode(1600, 1024), "SFML works!");

    ImGui::SFML::Init(window);

    ImGui::GetIO().Fonts->Clear();
    ImFont* font = ImGui::GetIO().Fonts->AddFontFromFileTTF("CONSOLA.TTF", 20);
    ImGui::SFML::UpdateFontTexture();

    Bus* bus = new Bus();
    //put this in first so that it maps first to controller port
    Controller* controller = new Controller(bus);
    Ram* ram = new Ram(bus);
    Bus* ppu_bus = new Bus();
    Cart* cart = new Cart(bus, ppu_bus);
    PPU* ppu =  new PPU(bus, ppu_bus);
    cart->load("roms/Elite (E).nes");
    CPU cpu(bus);

    window.setFramerateLimit(60);
    for (int i = 0; i < 21; i++) {
        ppu->step();
    }
    /*
    while(true) {
        cpu.step();
        if(cpu.did_fetch()) {
            cpu.write_debug(std::cout);
            std::cout << " PPU:" << std::setw(3) << std::right << std::setfill(' ') << ppu.line << ",";
            std::cout << std::setw(3) << std::right << std::setfill(' ') << ppu.px;
            std::cout << " CYC:" << cpu.cycle_count << std::endl;
            cycle_list << " CYC:" << cpu.cycle_count << std::endl;
            cpu.write_debug(log);
            log << " PPU:" << std::setw(3) << std::right << std::setfill(' ') << ppu.line << ",";
            log << std::setw(3) << std::right << std::setfill(' ') << ppu.px;
            log << " CYC:" << cpu.cycle_count << std::endl;

        }
        cpu.write_debug(cycle_log);
        cycle_log << " PPU:" << std::setw(3) << std::right << std::setfill(' ') << ppu.line << ",";
        cycle_log << std::setw(3) << std::right << std::setfill(' ') << ppu.px;
        cycle_log << " CYC:" << cpu.cycle_count << std::endl;
       
        ppu.step();
        ppu.step();
        ppu.step();
    }
    */

    

    sf::Clock deltaClock;
    bool even_odd = true;
    while (window.isOpen())
    {

        for(int i = 0; i < (29781); i++) {
            cpu.step();
            /*
            if (cpu.did_fetch()) {
                cpu.write_debug(std::cout);
                std::cout << " PPU:" << std::setw(3) << std::right << std::setfill(' ') << ppu.line << ",";
                std::cout << std::setw(3) << std::right << std::setfill(' ') << ppu.px;
                std::cout << " CYC:" << cpu.cycle_count << std::endl;
                cycle_list << " CYC:" << cpu.cycle_count << std::endl;
                cpu.write_debug(log);
                log << " PPU:" << std::setw(3) << std::right << std::setfill(' ') << ppu.line << ",";
                log << std::setw(3) << std::right << std::setfill(' ') << ppu.px;
                log << " CYC:" << cpu.cycle_count << std::endl;

            }
            cpu.write_debug(cycle_log);
            cycle_log << " PPU:" << std::setw(3) << std::right << std::setfill(' ') << ppu.line << ",";
            cycle_log << std::setw(3) << std::right << std::setfill(' ') << ppu.px;
            cycle_log << " CYC:" << cpu.cycle_count << std::endl;
            */
            ppu->step();
            ppu->step();
            ppu->step();
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed)
                window.close();
        }

        //std::cout << ppu.line << std::endl;
        window.setTitle("FPS: " + std::to_string(1.0 / deltaClock.getElapsedTime().asSeconds()));
        ImGui::SFML::Update(window, deltaClock.restart());

        window.clear();
        ImGui::ShowDemoWindow();

        if (ImGui::BeginMainMenuBar()) {

            if (ImGui::BeginMenu("Open")) {
                ImGuiFileDialog::Instance()->OpenDialog("OpenFileDlgKey", "Open", ".nes", ".");
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (ImGuiFileDialog::Instance()->Display("OpenFileDlgKey", NULL, ImVec2(300.0, 200.0))) {
            // action if OK
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                delete bus;
                delete ram;
                delete cart;
                delete ppu_bus;
                delete controller;
                
                bus = new Bus;
                //put this in first so that it maps first to controller port
                controller = new Controller(bus);
                ram = new Ram(bus);
                ppu_bus = new Bus;
                cart = new Cart(bus, ppu_bus);
                ppu = new PPU(bus, ppu_bus);
                cart->load(filePathName);
                cpu.reset(bus);
            }

            // close
            ImGuiFileDialog::Instance()->Close();
        }

        cart->draw_chr_rom(window, ppu);
        ppu->draw(window);
        ram->draw();
        cpu.draw_state(window);
        ImGui::SFML::Render(window);
        window.display();
    }

    return 0;
}
