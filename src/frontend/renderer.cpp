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
        init_pair(5, COLOR_BLUE, COLOR_BLACK);    // File information
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
    int screen_y = 0;
    int cy_pad = 0;     // Additional shift of cursor_y caused by screen lines
    for (int i = 0; i < screen_lines && i + top_line < static_cast<int>(lines.size()) && screen_y < screen_lines; ++i) {
        int logic_y = i + top_line + 1;
        // Render the line number of the logical line
        color_on(2);
        mvprintw(screen_y, 0, "%4d", logic_y);
        color_off(2);
        // Render text content
        const std::string& logical_line = lines[i + top_line];
        int line_length = logical_line.size();
        int start = 0;
        do {
            if (start != 0 && logic_y <= cursor_y)
                ++cy_pad;
            mvprintw(screen_y, 6, "%s", logical_line.substr(start, COLS - 6).c_str());
            start += (COLS - 6);    // Skip the rendered characters
            ++screen_y;
        } while (start < line_length && screen_y < screen_lines);
    }

    std::string mode_str = (mode == Mode::NORMAL) ? "-- NORMAL --" : 
                           (mode == Mode::INSERT) ? ">> INSERT <<" : ":: COMMAND ::";

    // Display status bar
    displayStatusBar(
        mode_str,
        filename.empty() ? "[No Name]" : "\"" + filename + "\", " + std::to_string(lines.size()) + "L",
        message,
        number_buffer,
        "("+std::to_string(cursor_y + 1)+", " + std::to_string(cursor_x + 1)+")"
    );

    // Move cursor to the correct position (limited in display area)
    int cys = cursor_x / (COLS - 6);
    int cursor_screen_x = (cursor_x % (COLS - 6)) + 6;
    int cursor_screen_y = cursor_y - top_line + cy_pad + cys;
    if (cursor_screen_y >= 0 && cursor_screen_y < screen_lines) {
        move(cursor_screen_y, cursor_screen_x); // 6 spaces for line numbers
    }

    refresh();
}

void Renderer::displayStatusBar(const std::string& mode, const std::string& fileInfos, const std::string& message, const std::string& cmd_buf, const std::string& coor) {
    if (message.empty()) {
        color_on(1);
        mvprintw(LINES - 1, 0, "%s", mode.c_str());
        mvprintw(LINES - 1, 16, "%s", coor.c_str());
        color_off(1);
    } else {
        color_on(4);
        mvprintw(LINES - 1, 0, "(%s)", message.c_str());
        color_off(4);
    }
    color_on(3);
    mvprintw(LINES - 1, COLS - fileInfos.size() - cmd_buf.size() - 16, "%s", cmd_buf.c_str());
    color_off(3);
    color_on(5);
    mvprintw(LINES - 1, COLS - fileInfos.size() - 1, "%s", fileInfos.c_str());
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

int Renderer::getCOLS() {return COLS;}
