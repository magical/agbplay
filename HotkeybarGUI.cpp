#include "HotkeybarGUI.h"
#include "ColorDef.h"
#include "MyException.h"
#include <ncurses.h>
#include <string>
#include <cstdint>

using namespace agbplay;

HotkeybarGUI::HotkeybarGUI(uint32_t height, uint32_t width,
        uint32_t yPos, uint32_t xPos) : CursesWin(height, width, yPos, xPos) {
    if (height == 0)
        throw MyException("Hotkeybar can't be empty");
    update();
}

HotkeybarGUI::~HotkeybarGUI() {
}

void HotkeybarGUI::update() {
    UIMutex.lock();
    wattrset(winPtr, COLOR_PAIR(GREEN_ON_DEFAULT) | A_REVERSE);
    // draw initial border
    std::string hkText = " [q=QUIT] [tab=SWITCH] [a=ADD] [d=DEL] [g=DRAG]";
    hkText.resize(width, ' ');
    mvwprintw(winPtr, 0, 0, hkText.c_str());
    
    std::string fillText = "";
    fillText.resize(width, ' ');
    for (uint32_t i = 1; i < height; i++) {
        mvwprintw(winPtr, (int)i, 0, fillText.c_str());
    }
    wrefresh(winPtr);
    UIMutex.unlock();
}

void HotkeybarGUI::Resize(uint32_t height, uint32_t width, uint32_t yPos, uint32_t xPos) {
    UIMutex.lock();
    if (delwin(winPtr) == ERR)
        UIMutex.unlock();
        throw MyException("Error while resizing Hotkeybar [delwin]");
    if ((winPtr = newwin((int)height, (int)width, (int)yPos, (int)xPos)) == nullptr)
        UIMutex.unlock();
        throw MyException("Error while resizing Hotkeybar [newwin]");
    this->height = height;
    this->width = width;
    update();
    UIMutex.unlock();
}