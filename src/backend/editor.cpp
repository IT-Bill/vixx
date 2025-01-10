// src/backend/editor.cpp

#include "backend/editor.h"
#include "common/utils.h"
#include "frontend/input_handler.h"
#include "frontend/renderer.h"
#include <stdexcept>
#include <string>

// Constructor
Editor::Editor()
    : mode(Mode::NORMAL), message(""), number_buffer(""),
      current_buffer_index(0), renderer(nullptr) {
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

// Adjust top_line for scrolling
void Editor::adjustScrolling() {
    int screen_lines = renderer->getScreenHeight() - 1;

    if (currentBuffer().getCursorY() < currentBuffer().getTopLine()) {
        // Scroll up
        currentBuffer().setTopLine(currentBuffer().getCursorY());
    } else {
        // Scroll down
        int new_top = currentBuffer().calculateTopLine(
            currentBuffer().getCursorY(), renderer->getCOLS(), screen_lines);
        if (new_top > currentBuffer().getTopLine()) {
            currentBuffer().setTopLine(new_top);
        }
    }
}

void Editor::refresh_render() {
    // Pull out references from the active buffer
    Buffer& buf = currentBuffer();
    renderer->render(buf, buf.getCursorX(), buf.getCursorY(), buf.getTopLine(),
                     mode, buf.getFilename(), message, number_buffer);
}

void Editor::clear_message() {
    message = "";
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

// Create a new buffer, or load existing file
void Editor::openFile(const std::string& fname) {
    Buffer buf;
    if (!fname.empty()) {
        buf.setFilename(fname);
        buf.loadFromFile(fname);
    }
    buffers.push_back(buf);
    current_buffer_index = (int)buffers.size() - 1;
    refresh_render();
}

// Switch to a different buffer by index
void Editor::switchBuffer(int index) {
    if (index >= 0 && index < (int)buffers.size()) {
        current_buffer_index = index;
        refresh_render();
    } else {
        message = "Invalid buffer number";
        refresh_render();
    }
}

// List all buffers
void Editor::listBuffers() {
    clear_message();
    message = "Buffers:\n";
    for (int i = 0; i < (int)buffers.size(); ++i) {
        message +=
            std::to_string(i + 1) + ": " + buffers[i].getFilename() + "\n";
    }
    // Potentially use your renderer to display or just store in 'message'
    // We'll store in 'message' for simplicity:
    refresh_render();
}

Buffer& Editor::currentBuffer() {
    // Always assume currentBufferIndex >= 0
    return buffers[current_buffer_index];
}
const Buffer& Editor::currentBuffer() const {
    return buffers[current_buffer_index];
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
    currentBuffer().moveCursorLeft(t);
    refresh_render();
}
void Editor::moveCursorRight(int t) {
    currentBuffer().moveCursorRight(t);
    refresh_render();
}
void Editor::moveCursorUp(int t) {
    currentBuffer().moveCursorUp(t);
    adjustScrolling();
    refresh_render();
}
void Editor::moveCursorDown(int t) {
    currentBuffer().moveCursorDown(t);
    adjustScrolling();
    refresh_render();
}

// Jump to the beginning of the current line
void Editor::jumpToLineStart() {
    currentBuffer().jumpToLineStart();
    refresh_render();
}

// Jump to the end of the current line
void Editor::jumpToLineEnd() {
    currentBuffer().jumpToLineEnd();
    refresh_render();
}

// Go to the first line with the cursor at the beginning
void Editor::goToFirstLine() {
    currentBuffer().goToFirstLine();
    adjustScrolling();
    refresh_render();
}

// Go to the last line with the cursor at the beginning
void Editor::goToLastLine() {
    currentBuffer().goToLastLine();
    adjustScrolling();
    refresh_render();
}

void Editor::jumpToLine(int target_line) {
    currentBuffer().jumpToLine(target_line);
    adjustScrolling();
    refresh_render();
}

// Delete the current line
void Editor::deleteCurrentLine() {
    currentBuffer().deleteCurrentLine();
    adjustScrolling();
    refresh_render();
}

// Copy the current line
void Editor::copyCurrentLine() {
    // Save the current line to the clipboard
    copied_line = currentBuffer().getLine(
        currentBuffer().getCursorY()); 
}

// Paste the copied content
void Editor::pasteContent(int t) {
    currentBuffer().pasteContent(copied_line, t);
    adjustScrolling();
    refresh_render();
}

// Insert Mode Operations
void Editor::insertCharacter(char c) {
    currentBuffer().insertCharacter(c);
    refresh_render();
}

void Editor::handleBackspace() {
    currentBuffer().handleBackspace();
    adjustScrolling();
    refresh_render();
}

void Editor::handleEnter() {
    currentBuffer().handleEnter();
    adjustScrolling();
    refresh_render();
}

// ===--- Command Execution ---===
void Editor::executeCommand(const std::string& command) {
    // split the command into parts
    std::vector<std::string> parts = split(command, 2);

    // process the write command with optional filename
    if (parts[0] == "e") {
        if (parts.size() > 1) {
            openFile(parts[1]);
        } else {
            message = "No file specified";
        }
    }
    // "ls" to list buffers
    else if (parts[0] == "ls") {
        listBuffers();
    }
    // "buffer <n>" to switch to the nth buffer
    else if (parts[0] == "buffer" || parts[0] == "b") {
        if (parts.size() > 1) {
            int idx = std::stoi(parts[1]) - 1; // user is 1-based
            switchBuffer(idx);
        } else {
            message = "buffer command requires a number";
        }
    } else if (parts[0] == "w") {
        try {
            saveFile(parts.size() > 1 ? parts[1] : "");
        } catch (const std::runtime_error& e) {
            message = e.what();
        }

    } else if (parts[0] == "q") {
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

    } else if (command.rfind("s/", 0) == 0) { // s/old/new/g
        size_t pref = 1;
        size_t first = command.find('/', pref + 1);
        size_t second = command.find('/', first + 1);
        if (first != std::string::npos && second != std::string::npos) {
            std::string old_str = command.substr(pref + 1, first - pref - 1);
            std::string new_str = command.substr(first + 1, second - first - 1);
            currentBuffer().replaceAll(old_str, new_str);
            
            refresh_render();
        } else {
            message = "Insufficient parameter";
        }

    } else {
        message = "Not an editor command: " + command;
    }
    
}

// ===--- Undo/Redo Operations ---===
void Editor::undo() {
    currentBuffer().undo();
    refresh_render();
}

void Editor::redo() {
    currentBuffer().redo();
    refresh_render();
}

void Editor::saveFile(const std::string& fname) {
    currentBuffer().saveToFile(fname);
    // Optionally, display a save confirmation in the status bar
    refresh_render();
}

// Renderer Access
Renderer& Editor::getRenderer() {
    return *renderer;
}
