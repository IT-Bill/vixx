// src/frontend/renderer.cpp

#include "frontend/renderer.h"
#include <ncurses.h>

Renderer::Renderer() : colors_initialized(false) {}

Renderer::~Renderer() {}

void Renderer::initialize() {
    initscr();              // Initialize the window
    cbreak();               // Disable line buffering
    noecho();               // Don't echo pressed keys
    keypad(stdscr, TRUE);   // Enable function keys and arrow keys
    curs_set(1);            // Show the cursor

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Normal mode
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Insert mode
        init_pair(3, COLOR_CYAN, COLOR_BLACK);    // Command mode
        colors_initialized = true;
    }
}

void Renderer::shutdown() {
    endwin();
}

void Renderer::render(const Buffer& buffer, int cursor_x, int cursor_y, Mode mode, const std::string& filename) {
    clear();

    // Display buffer lines with line numbers
    const auto& lines = buffer.getLines();
    for (int i = 0; i < lines.size(); ++i) {
        // Display line numbers (right-aligned in 4 spaces)
        mvprintw(i, 0, "%4d: %s", i + 1, lines[i].c_str());
    }

    // Display status bar
    displayStatusBar(
        (mode == Mode::NORMAL) ? "NORMAL" : 
        (mode == Mode::INSERT) ? "INSERT" : "COMMAND",
        filename.empty() ? "[No Name]" : filename
    );

    // Move cursor to the correct position
    move(cursor_y, cursor_x + 6); // 5 spaces for line numbers and 1 for ':'

    refresh();
}

void Renderer::displayStatusBar(const std::string& mode, const std::string& filename) {
    if (colors_initialized) {
        attron(COLOR_PAIR(1)); // Normal color
    }
    mvprintw(LINES - 1, 0, "-- %s -- %s", mode.c_str(), filename.c_str());
    if (colors_initialized) {
        attroff(COLOR_PAIR(1));
    }
}

void Renderer::displayCommandLine(const std::string& command) {
    if (colors_initialized) {
        attron(COLOR_PAIR(3)); // Command mode color
    }
    mvprintw(LINES - 1, 0, ":%s", command.c_str());
    if (colors_initialized) {
        attroff(COLOR_PAIR(3));
    }
}

void Renderer::clearCommandLine() {
    move(LINES - 1, 0);
    clrtoeol();
}
