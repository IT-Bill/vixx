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
    set_escdelay(50);       // Set ESC latency (milliseconds)

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Status bar
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Line number
        init_pair(3, COLOR_CYAN, COLOR_BLACK);    // Command
        init_pair(4, COLOR_WHITE, COLOR_RED);     // Message
        init_pair(5, COLOR_BLUE, COLOR_BLACK);    // Coordinate
        colors_initialized = true;
    }
}

void Renderer::shutdown() {
    endwin();
}

int Renderer::getScreenHeight() const {
    return LINES; // Number of screen lines
}

void Renderer::render(const Buffer& buffer, int cursor_x, int cursor_y, int top_line, Mode mode, const std::string& filename, const std::string& message, const std::string& number_buffer) {
    clear();

    // Display line numbers and buffer lines
    const auto& lines = buffer.getLines();
    int screen_lines = LINES - 1; // Reserve space for status bar
    for (int i = 0; i < screen_lines && i + top_line < static_cast<int>(lines.size()); ++i) {
        color_on(2);
        mvprintw(i, 0, "%4d", i + top_line + 1);
        color_off(2);
        mvprintw(i, 6, "%s", lines[i + top_line].c_str());
    }

    // Display status bar
    displayStatusBar(
        (mode == Mode::NORMAL) ? "NORMAL" : 
        (mode == Mode::INSERT) ? "INSERT" : "COMMAND",
        filename.empty() ? "[No Name]" : filename,
        message,
        number_buffer,
        "("+std::to_string(cursor_y + 1)+", " + std::to_string(cursor_x + 1)+")"
    );

    // Move cursor to the correct position (limited in display area)
    int cursor_screen_y = cursor_y - top_line;
    if (cursor_screen_y >= 0 && cursor_screen_y < screen_lines) {
        move(cursor_screen_y, cursor_x + 6); // 6 spaces for line numbers
    }

    refresh();
}

void Renderer::displayStatusBar(const std::string& mode, const std::string& filename, const std::string& message, const std::string& cmd_buf, const std::string& coor) {
    if (message.empty()) {
        color_on(1);
        mvprintw(LINES - 1, 0, "-- %s -- %s", mode.c_str(), filename.c_str());
        color_off(1);
    } else {
        color_on(4);
        mvprintw(LINES - 1, 0, "(%s)", message.c_str());
        color_off(4);
    }
    color_on(3);
    mvprintw(LINES - 1, COLS - coor.size() - cmd_buf.size() - 8, "%s", cmd_buf.c_str());
    color_off(3);
    color_on(5);
    mvprintw(LINES - 1, COLS - coor.size() - 1, "%s", coor.c_str());
    color_off(5);
}

void Renderer::displayCommandLine(const std::string& command) {
    clearCommandLine();
    color_on(3);
    mvprintw(LINES - 1, 0, ":%s", command.c_str());
    color_off(3);
}

void Renderer::clearCommandLine() {
    move(LINES - 1, 0);
    clrtoeol();
}

void Renderer::color_on(int order) {if (colors_initialized) attron(COLOR_PAIR(order));}
void Renderer::color_off(int order) {if (colors_initialized) attroff(COLOR_PAIR(order));}
