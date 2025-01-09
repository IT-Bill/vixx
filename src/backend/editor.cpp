// src/backend/editor.cpp

#include "backend/editor.h"
#include "frontend/renderer.h"
#include "frontend/input_handler.h"
#include "common/utils.h"
#include <stdexcept>
#include <string>

// Constructor
Editor::Editor() : mode(Mode::NORMAL), cursor_x(0), cursor_y(0), top_line(0), filename(""), message(""), number_buffer(""), renderer(nullptr) {
    initialize();
}

// Destructor
Editor::~Editor() {
    shutdown();
}

void Editor::refresh_render() {
    renderer->render(buffer, cursor_x, cursor_y, top_line, mode, filename, message, number_buffer);
}

void Editor::clear_message() {
    message = "";
}

// Initialize the editor (including the renderer)
void Editor::initialize() {
    renderer = new Renderer();
    renderer->initialize();
    // Initialize buffer with at least one empty line
    // buffer.addLine("");
}

// Shutdown the editor and renderer
void Editor::shutdown() {
    if (renderer) {
        renderer->shutdown();
        delete renderer;
        renderer = nullptr;
    }
}

// Mode Management
Mode Editor::getMode() const {
    return mode;
}

void Editor::switchMode(Mode new_mode) {
    mode = new_mode;
    if (mode != Mode::COMMAND) {
        renderer->clearCommandLine();
    }
    refresh_render();
}

// Adjust top_line for scrolling
void Editor::adjustScrolling() {
    int screen_lines = renderer->getScreenHeight() - 1;
    if (cursor_y < top_line) {  // Scroll up
        top_line = cursor_y;
    } else {    // Scroll down
        int new_top = buffer.calculateTopLine(cursor_y, renderer->getCOLS(), screen_lines);
        if (new_top > top_line)
            top_line = new_top;
    } 
}

std::string& Editor::getNumberBuffer() {
    return number_buffer;
}
void Editor::appendNumberBuffer(const char ch) {
    number_buffer += ch;
}
void Editor::clearNumberBuffer() {
    number_buffer.clear();
}

// Cursor Movement
void Editor::moveCursorLeft(int t) {
    cursor_x -= t;
    int min = 0;
    if (cursor_x < min)
        cursor_x = min;
    refresh_render();
}
void Editor::moveCursorRight(int t) {
    cursor_x += t;
    int max = static_cast<int>(buffer.getLine(cursor_y).size());
    if (cursor_x > max)
        cursor_x = max;
    refresh_render();
}
void Editor::moveCursorUp(int t) {
    cursor_y -= t;
    int min = 0;
    if (cursor_y < min)
        cursor_y = min;
    if (cursor_x > static_cast<int>(buffer.getLine(cursor_y).size()))
        cursor_x = buffer.getLine(cursor_y).size();
    adjustScrolling();
    refresh_render();
}
void Editor::moveCursorDown(int t) {
    cursor_y += t;
    int max = buffer.getLineCount() - 1;
    if (cursor_y > max)
        cursor_y = max;
    if (cursor_x > static_cast<int>(buffer.getLine(cursor_y).size()))
        cursor_x = buffer.getLine(cursor_y).size();
    adjustScrolling();
    refresh_render();
}

// Jump to the beginning of the current line
void Editor::jumpToLineStart() {
    cursor_x = 0;
    refresh_render();
}

// Jump to the end of the current line
void Editor::jumpToLineEnd() {
    cursor_x = buffer.getLine(cursor_y).size();
    refresh_render();
}

// Go to the first line with the cursor at the beginning
void Editor::goToFirstLine() {
    cursor_y = 0;
    cursor_x = 0;
    adjustScrolling();
    refresh_render();
}

// Go to the last line with the cursor at the beginning
void Editor::goToLastLine() {
    cursor_y = buffer.getLineCount() - 1;
    cursor_x = 0;
    adjustScrolling();
    refresh_render();
}

void Editor::jumpToLine(int target_line) {
    if (target_line < 0) 
        target_line = 0;
    else if (target_line >= buffer.getLineCount())
        target_line = buffer.getLineCount() - 1;
    cursor_y = target_line;
    cursor_x = 0;
    adjustScrolling();
    refresh_render();
}


// Delete the current line
void Editor::deleteCurrentLine() {
    // Record action for undo
    Action action;
    action.type = Action::DELETE;
    action.line = cursor_y;
    action.pos = 0;
    action.text = std::string(buffer.getLine(cursor_y) + "\n");
    undo_stack.push(action);

    buffer.deleteLine(cursor_y);
    if (cursor_y >= buffer.getLineCount()) {
        cursor_y = buffer.getLineCount() - 1; // Adjust if the cursor is beyond the last line
    }

    cursor_x = 0; // Move cursor to the beginning of the line
    adjustScrolling();
    refresh_render();
}

// Copy the current line
void Editor::copyCurrentLine() {
    copied_line = buffer.getLine(cursor_y); // Save the current line to the clipboard
}

// Paste the copied content
void Editor::pasteContent(int t) {
    if (!copied_line.empty()) {
        for (int i = 0; i < t; ++i) {
            std::string& current_line = buffer.getLine(cursor_y);
            std::string before_cursor = current_line.substr(0, cursor_x);
            std::string after_cursor = current_line.substr(cursor_x);

            current_line = before_cursor + copied_line + after_cursor;
            cursor_x += copied_line.size();
        }
        refresh_render();
    }
}


// Insert Mode Operations
void Editor::insertCharacter(char c) {
    buffer.insertChar(cursor_y, cursor_x, c);
    // Record action for undo
    Action action;
    action.type = Action::INSERT;
    action.line = cursor_y;
    action.pos = cursor_x;
    action.text = std::string(1, c);
    undo_stack.push(action);
    // Update cursor position
    cursor_x++;
    refresh_render();
}

void Editor::handleBackspace() {
    if (cursor_x > 0) {
        char deleted_char = buffer.getLine(cursor_y)[cursor_x - 1];
        buffer.deleteChar(cursor_y, cursor_x - 1);
        // Record action for undo
        Action action;
        action.type = Action::DELETE;
        action.line = cursor_y;
        action.pos = cursor_x - 1;
        action.text = std::string(1, deleted_char);
        undo_stack.push(action);
        // Update cursor position
        cursor_x--;
    }
    else if (cursor_y > 0) {
        // Merge with previous line
        int prev_line_length = buffer.getLine(cursor_y - 1).size();
        std::string current_line = buffer.getLine(cursor_y);
        buffer.mergeLines(cursor_y - 1, prev_line_length);
        // Record action for undo (line merge)
        Action action;
        action.type = Action::DELETE;
        action.line = cursor_y - 1;
        action.pos = prev_line_length;
        action.text = "\n"; // Representing line merge
        undo_stack.push(action);
        // Update cursor position
        cursor_y--;
        cursor_x = prev_line_length;
    }
    adjustScrolling();
    refresh_render();
}

void Editor::handleEnter() {
    buffer.splitLine(cursor_y, cursor_x);
    // Record action for undo (line split)
    Action action;
    action.type = Action::INSERT;
    action.line = cursor_y;
    action.pos = cursor_x;
    action.text = "\n"; // Representing line split
    undo_stack.push(action);
    // Move to the new line
    cursor_y++;
    cursor_x = 0;
    adjustScrolling();
    refresh_render();
}

// Command Execution
void Editor::executeCommand(const std::string& command) {
    // split the command into parts
    std::vector<std::string> parts = split(command, 2);

    // process the write command with optional filename
    if (parts[0] == "w") {
        try {
            saveFile(parts.size() > 1 ? parts[1] : "");
        } catch (const std::runtime_error& e) {
            message = e.what();
        }
    }
    else if (parts[0] == "q") {
        shutdown();
        exit(0);
    }
    // process the write command with optional filename
    else if (parts[0] == "wq") {
        try {
            saveFile(parts.size() > 1 ? parts[1] : "");
            shutdown();
            exit(0);
        } catch (const std::runtime_error& e) {
            message = e.what();
        }

    }
    else if (command.rfind("s/", 0) == 0) { // s/old/new/g
        size_t pref = 1;
        size_t first = command.find('/', pref + 1);
        size_t second = command.find('/', first + 1);
        if (first != std::string::npos && second != std::string::npos) {
            std::string old_str = command.substr(pref + 1, first - pref - 1);
            std::string new_str = command.substr(first + 1, second - first - 1);
            for (int i = 0; i < buffer.getLineCount(); ++i) {
                buffer.replaceAll(i, old_str, new_str);
            }
            // Optionally, record replace action for undo
            refresh_render();
        } else {
            message = "Insufficient parameter";
        }
    } else {
        message = "Not an editor command: " + command;
    }
    // Add more commands as needed
}

// Undo/Redo Operations
void Editor::undo() {
    if (undo_stack.empty()) return;
    Action action = undo_stack.top();
    undo_stack.pop();

    switch (action.type) {
        case Action::INSERT:
            if (action.text == "\n") {
                // We inserted a newline => we splitted a line. 
                // Undo means we do the opposite: merge lines.
                buffer.mergeLines(action.line, action.pos);

                // Position the cursor so it makes sense after the merge.
                cursor_y = action.line;
                cursor_x = action.pos;
            } else {
                // We inserted a character => undo means we delete it.
                buffer.deleteChar(action.line, action.pos);

                // Position the cursor so it makes sense after the deletion.
                cursor_y = action.line;
                cursor_x = action.pos;
            }
            break;
            
        case Action::DELETE:
            if (action.text == "\n") {
                // We deleted a newline => we merged lines. 
                // Undo means we do the opposite: split lines.
                buffer.splitLine(action.line, action.pos);

                // Position the cursor so it makes sense after the split.
                cursor_y = action.line + 1; 
                cursor_x = 0; // or you can refine this 
            } else {
                // We deleted a normal character => re-insert that char.
                buffer.insertChar(action.line, action.pos, action.text[0]);
                cursor_y = action.line;
                cursor_x = action.pos + 1;
            }
            break;
        case Action::REPLACE:
            // Implement replace undo if needed
            break;
    }

    // Push the inverse action to redo stack
    redo_stack.push(action);
    refresh_render();
}

void Editor::redo() {
    if (redo_stack.empty()) return;
    Action action = redo_stack.top();
    redo_stack.pop();

    switch (action.type) {
        case Action::INSERT:
            if (action.text == "\n") {
                // We re-insert a newline => line split again.
                buffer.splitLine(action.line, action.pos);
                cursor_y = action.line + 1;
                cursor_x = 0; // or refine further
            } else {
                // Re-insert the character
                buffer.insertChar(action.line, action.pos, action.text[0]);
                cursor_x = action.pos + 1;
                cursor_y = action.line;
            }
            break;

        case Action::DELETE:
            if (action.text == "\n") {
                // We re-delete a newline => line merge again.
                buffer.mergeLines(action.line, action.pos);

                // After merging, the new line disappears, so...
                cursor_y = action.line;
                cursor_x = action.pos;
            } else {
                // Remove the character again
                buffer.deleteChar(action.line, action.pos);
                cursor_x = action.pos;
                cursor_y = action.line;
            }
            break;
            
        case Action::REPLACE:
            // Implement replace redo if needed
            break;
    }

    // Push the action back to undo stack
    undo_stack.push(action);
    refresh_render();
}

// File Operations
void Editor::openFile(const std::string& fname) {
    filename = fname;
    if (!buffer.loadFromFile(fname)) {
        // If file doesn't exist, start with an empty buffer
        // buffer.addLine("");
    }
    refresh_render();
}

void Editor::saveFile(const std::string& fname) {
    if (filename.empty() && fname.empty()) {
        throw std::runtime_error("No filename specified");
    } else if (!fname.empty()) {
        filename = fname;
    }
    buffer.saveToFile(filename);
    // Optionally, display a save confirmation in the status bar
    refresh_render();
}

// Renderer Access
Renderer& Editor::getRenderer() {
    return *renderer;
}
