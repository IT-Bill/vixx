// src/backend/editor.cpp

#include "backend/editor.h"
#include "frontend/renderer.h"
#include "frontend/input_handler.h"
#include <iostream>

// Constructor
Editor::Editor() : mode(Mode::NORMAL), cursor_x(0), cursor_y(0), filename(""), renderer(nullptr) {
    initialize();
}

// Destructor
Editor::~Editor() {
    shutdown();
}

// Initialize the editor (including the renderer)
void Editor::initialize() {
    renderer = new Renderer();
    renderer->initialize();
    // Initialize buffer with at least one empty line
    buffer.addLine("");
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
    renderer->render(buffer, cursor_x, cursor_y, mode, filename);
}

// Cursor Movement
void Editor::moveCursorLeft() {
    if (cursor_x > 0) {
        cursor_x--;
    }
    renderer->render(buffer, cursor_x, cursor_y, mode, filename);
}

void Editor::moveCursorRight() {
    if (cursor_x < static_cast<int>(buffer.getLine(cursor_y).size())) {
        cursor_x++;
    }
    renderer->render(buffer, cursor_x, cursor_y, mode, filename);
}

void Editor::moveCursorUp() {
    if (cursor_y > 0) {
        cursor_y--;
        if (cursor_x > static_cast<int>(buffer.getLine(cursor_y).size())) {
            cursor_x = buffer.getLine(cursor_y).size();
        }
    }
    renderer->render(buffer, cursor_x, cursor_y, mode, filename);
}

void Editor::moveCursorDown() {
    if (cursor_y < buffer.getLineCount() - 1) {
        cursor_y++;
        if (cursor_x > static_cast<int>(buffer.getLine(cursor_y).size())) {
            cursor_x = buffer.getLine(cursor_y).size();
        }
    }
    renderer->render(buffer, cursor_x, cursor_y, mode, filename);
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
    renderer->render(buffer, cursor_x, cursor_y, mode, filename);
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
    renderer->render(buffer, cursor_x, cursor_y, mode, filename);
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
    renderer->render(buffer, cursor_x, cursor_y, mode, filename);
}

// Command Execution
void Editor::executeCommand(const std::string& command) {
    if (command == "w") {
        saveFile();
    }
    else if (command == "q") {
        shutdown();
        exit(0);
    }
    else if (command == "wq") {
        saveFile();
        shutdown();
        exit(0);
    }
    else if (command.rfind("s/", 0) == 0) { // s/old/new/g
        size_t first = command.find('/', 2);
        size_t second = command.find('/', first + 1);
        size_t third = command.find('/', second + 1);
        if (first != std::string::npos && second != std::string::npos) {
            std::string old_str = command.substr(first + 1, second - first - 1);
            std::string new_str;
            size_t end = command.find('/', second + 1);
            if (end != std::string::npos) {
                new_str = command.substr(second + 1, end - second - 1);
            } else {
                new_str = command.substr(second + 1);
            }
            buffer.replaceAll(cursor_y, old_str, new_str);
            // Optionally, record replace action for undo
        }
    }
    // Add more commands as needed

    renderer->render(buffer, cursor_x, cursor_y, mode, filename);
}

// Undo/Redo Operations
void Editor::undo() {
    if (undo_stack.empty()) return;
    Action action = undo_stack.top();
    undo_stack.pop();

    switch (action.type) {
        case Action::INSERT:
            // Remove the inserted character
            buffer.deleteChar(action.line, action.pos);
            cursor_x = action.pos;
            cursor_y = action.line;
            break;
        case Action::DELETE:
            if (action.text == "\n") {
                // Handle line merge
                buffer.mergeLines(action.line, action.pos);
                cursor_y = action.line + 1;
                cursor_x = 0;
            }
            else {
                // Re-insert the deleted character
                buffer.insertChar(action.line, action.pos, action.text[0]);
                cursor_x = action.pos + 1;
                cursor_y = action.line;
            }
            break;
        case Action::REPLACE:
            // Implement replace undo if needed
            break;
    }

    // Push the inverse action to redo stack
    redo_stack.push(action);
    renderer->render(buffer, cursor_x, cursor_y, mode, filename);
}

void Editor::redo() {
    if (redo_stack.empty()) return;
    Action action = redo_stack.top();
    redo_stack.pop();

    switch (action.type) {
        case Action::INSERT:
            // Re-insert the character
            buffer.insertChar(action.line, action.pos, action.text[0]);
            cursor_x = action.pos + 1;
            cursor_y = action.line;
            break;
        case Action::DELETE:
            if (action.text == "\n") {
                // Handle line split
                buffer.splitLine(action.line, action.pos);
                cursor_y = action.line + 1;
                cursor_x = 0;
            }
            else {
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
    renderer->render(buffer, cursor_x, cursor_y, mode, filename);
}

// File Operations
void Editor::openFile(const std::string& fname) {
    filename = fname;
    if (!buffer.loadFromFile(fname)) {
        // If file doesn't exist, start with an empty buffer
        buffer.addLine("");
    }
    renderer->render(buffer, cursor_x, cursor_y, mode, filename);
}

void Editor::saveFile() {
    if (filename.empty()) {
        // Prompt for filename if not set
        // For simplicity, we'll assume filename is set
        return;
    }
    buffer.saveToFile(filename);
    // Optionally, display a save confirmation in the status bar
    renderer->render(buffer, cursor_x, cursor_y, mode, filename);
}

// Renderer Access
Renderer& Editor::getRenderer() {
    return *renderer;
}
