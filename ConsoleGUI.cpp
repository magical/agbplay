#include <string>

#include "ConsoleGUI.h"
#include "Debug.h"
#include "WindowGUI.h"
#include "MyException.h"
#include "ColorDef.h"

using namespace agbplay;
using namespace std;

/*
 * -- public --
 */

ConsoleGUI::ConsoleGUI(uint32_t height, uint32_t width,
        uint32_t yPos, uint32_t xPos) : CursesWin(height, width, yPos, xPos) {
    UIMutex.lock();
    keypad(winPtr, true);
    UIMutex.unlock();
    textWidth = width - 2;
    textHeight = height;
    textBuffer = vector<string>();
    update();
}

ConsoleGUI::~ConsoleGUI() {
}

void ConsoleGUI::Resize(uint32_t height, uint32_t width,
        uint32_t yPos, uint32_t xPos) {
    CursesWin::Resize(height, width, yPos, xPos);
    UIMutex.lock();
    keypad(winPtr, true);
    UIMutex.unlock();
    textHeight = height;
    textWidth = width - 2;
    update();    
}

void ConsoleGUI::WriteLn(string str) {
    // shift buffer
    writeToBuffer(str);
    update();
}

int ConsoleGUI::ConGetCH() {
    UIMutex.lock();
    int ret = wgetch(winPtr);
    UIMutex.unlock();
    return ret;
}

/*
 * -- private --
 */

void ConsoleGUI::update() {
    UIMutex.lock();
    for (uint32_t i = 0; i < textHeight; i++) {
        wattrset(winPtr, COLOR_PAIR(GREEN_ON_DEFAULT) | A_REVERSE);
        mvwprintw(winPtr, (int)i, 0, ">");
        wattrset(winPtr, COLOR_PAIR(DEFAULT_ON_DEFAULT) | A_REVERSE);
        mvwprintw(winPtr, (int)i, 1, " ");
        string txt;
        if (i < textBuffer.size()) {
            txt = textBuffer.at(i);
        } else {
            txt = "";
        }
        txt.resize(textWidth, ' ');
        mvwprintw(winPtr, (int)i, 2, txt.c_str());
    }
    wrefresh(winPtr);
    UIMutex.unlock();
}

void ConsoleGUI::writeToBuffer(string str) {
    textBuffer.push_back(str);
    if (textBuffer.size() > textHeight) {
        textBuffer.erase(textBuffer.begin() + 0);
    }
}
