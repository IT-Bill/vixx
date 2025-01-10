// src/frontend/renderer.cpp

#include "frontend/renderer.h"
#include <ncurses.h>
#include <string>

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
        init_pair(6, COLOR_BLACK, COLOR_WHITE);   // Active Tab
        colors_initialized = true;
    }
}

void Renderer::shutdown() {
    endwin();
}

int Renderer::getScreenHeight() const {
    return LINES; // Number of screen lines
}

void Renderer::render(const std::vector<Buffer>& buffers, int current_buffer_index,
                     int cursor_x, int cursor_y, int top_line, Mode mode,
                     const std::string& message, const std::string& number_buffer) {
    clear();
    
    // Render Tab Bar if multiple buffers are open
    if (buffers.size() > 1) {
        renderTabBar(buffers, current_buffer_index);
    }

    // Display line numbers and buffer lines
    const Buffer& current_buffer = buffers[current_buffer_index];
    const auto& lines = current_buffer.getLines();
    int screen_lines = LINES - ((buffers.size() > 1) ? 2 : 1);   // Reserve space for tab bar and status bar
    int screen_y = (buffers.size() > 1) ? 1 : 0;                 // Start from line 1 to leave space for tab bar
    int cy_pad = 0;                                              // Additional shift of cursor_y caused by screen lines

    for (int i = 0; i < screen_lines && i + top_line < static_cast<int>(lines.size()) && screen_y < screen_lines + ((buffers.size() > 1) ? 1 : 0); ++i) {
        int logic_y = i + top_line + 1;
        // Render the line number of the logical line
        color_on(2);
        mvprintw(screen_y, 0, "%4d", logic_y); // 1-based numbering
        color_off(2);
        // Render text content
        const std::string& logical_line = lines[i + top_line];
        int line_length = logical_line.size();
        int start = 0;
        do {
            if (start != 0 && logic_y - 1 <= cursor_y) // Cursor is in the current line
                ++cy_pad;
            mvprintw(screen_y, 6, "%s", logical_line.substr(start, COLS - 6).c_str());
            start += (COLS - 6);    // Skip the rendered characters
            ++screen_y;
        } while (start < line_length && screen_y < screen_lines + ((buffers.size() > 1) ? 1 : 0));
    }

    // Display status bar
    std::string fileInfos = current_buffer.getFilename().empty() ? "[No Name]" : "\"" + current_buffer.getFilename() + "\", " + std::to_string(lines.size()) + "L";
    std::string coor = "(" + std::to_string(cursor_y + 1) + ", " + std::to_string(cursor_x + 1) + ")";
    std::string mode_str = (mode == Mode::NORMAL) ? "-- NORMAL --" : 
                           (mode == Mode::INSERT) ? ">> INSERT <<" : ":: COMMAND ::";
    displayStatusBar(
        mode_str,
        fileInfos,
        message,
        number_buffer,
        coor
    );

    // Move cursor to the correct position (limited in display area)
    int cys = cursor_x / (COLS - 6);
    int cursor_screen_x = (cursor_x % (COLS - 6)) + 6;
    int cursor_screen_y = cursor_y - top_line + cy_pad + ((buffers.size() > 1) ? 1 : 0);
    if (cursor_screen_y >= 0 && cursor_screen_y < screen_lines + ((buffers.size() > 1) ? 1 : 0)) {
        move(cursor_screen_y, cursor_screen_x); // 6 spaces for line numbers
    }

    refresh();
}

void Renderer::renderTabBar(const std::vector<Buffer>& buffers, int current_buffer_index) {
    // Render each buffer as a tab
    int x = 0;
    for (int i = 0; i < buffers.size(); ++i) {
        const Buffer& buf = buffers[i];
        std::string tab_name = "[" + std::to_string(i + 1) + "] " + (buf.getFilename().empty() ? "[No Name]" : buf.getFilename()) + " ";
        if (i == current_buffer_index) {
            // Highlight active tab
            color_on(6);
            mvprintw(0, x, "%s", tab_name.c_str());
            color_off(6);
        } else {
            // Inactive tabs
            mvprintw(0, x, "%s", tab_name.c_str());
        }
        x += tab_name.size() + 1; // +1 for space
    }
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
