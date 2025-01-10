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
      current_tab_index(0), renderer(nullptr) {
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
    // openTab();
}

// Shutdown the editor and renderer
void Editor::shutdown() {
    if (renderer) {
        renderer->shutdown();
        delete renderer;
        renderer = nullptr;
    }
}

// Open a new tab with an optional filename
void Editor::openTab(const std::string& filename) {
    tabs.emplace_back(filename);
    current_tab_index = tabs.size() - 1;
    refreshRender();
}

// Close the current tab
void Editor::closeCurrentTab() {
    if (tabs.empty()) return;

    Tab& tab = currentTab();
    if (tab.isModified()) {
        message = "No write since last change (add ! to override)";
        refreshRender();
        return;
    }

    tabs.erase(tabs.begin() + current_tab_index);
    if (current_tab_index >= tabs.size()) {
        current_tab_index = tabs.size() - 1;
    }

    if (tabs.empty()) {
        shutdown(); // Exit the editor if no tabs are open
        exit(0);
    }

    refreshRender();
}

void Editor::switchToNextTab() {
    if (tabs.size() <= 1) return;
    current_tab_index = (current_tab_index + 1) % tabs.size();
    refreshRender();
}

void Editor::switchToPreviousTab() {
    if (tabs.size() <= 1) return;
    current_tab_index = (current_tab_index - 1 + tabs.size()) % tabs.size();
    refreshRender();
}

void Editor::switchToTab(int index) {
    if (index >= 0 && index < (int)tabs.size()) {
        current_tab_index = index;
        refreshRender();
    } else {
        message = "Invalid tab number";
        refreshRender();
    }
}

// List all open tabs
void Editor::listTabs() {
    clearMessage();
    message = "Tabs:\n";
    for (int i = 0; i < static_cast<int>(tabs.size()); ++i) {
        message +=
            std::to_string(i + 1) + ": " + tabs[i].getFilename() +
            (tabs[i].isModified() ? " +" : "") + "\n";
    }
    refreshRender();
}

// Get the current tab
Tab& Editor::currentTab() {
    return tabs[current_tab_index];
}

const Tab& Editor::currentTab() const {
    return tabs[current_tab_index];
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
    refreshRender();
}

// Adjust top_line for scrolling
void Editor::adjustScrolling() {
    int screen_lines = renderer->getScreenHeight() - 1;

    if (currentTab().getCursorY() < currentTab().getTopLine()) {
        // Scroll up
        currentTab().setTopLine(currentTab().getCursorY());
    } else {
        // Scroll down
        int new_top = currentTab().calculateTopLine(
            currentTab().getCursorY(), renderer->getCOLS(), screen_lines);
        if (new_top > currentTab().getTopLine()) {
            currentTab().setTopLine(new_top);
        }
    }
}

void Editor::refreshRender() {
    // Pull out references from the active buffer
    Tab& tab = currentTab();
    renderer->render(tab.getBuffer(), tab.getCursorX(), tab.getCursorY(), tab.getTopLine(),
                     mode, tab.getFilename(), message, number_buffer);
}

void Editor::setMessage(const std::string& msg) {
    message = msg;
}

void Editor::clearMessage() {
    message = "";
}



// Create a new buffer, or load existing file
void Editor::openFile(const std::string& fname) {
    Tab tab;
    if (!fname.empty()) {
        tab.setFilename(fname);
        tab.getBuffer().loadFromFile(fname);
    }
    tabs.push_back(tab);
    current_tab_index = (int)tabs.size() - 1;
    refreshRender();
}

// Switch to a different buffer by index
void Editor::switchBuffer(int index) {
    if (index >= 0 && index < (int)tabs.size()) {
        current_tab_index = index;
        refreshRender();
    } else {
        message = "Invalid buffer number";
        refreshRender();
    }
}

// List all buffers
void Editor::listBuffers() {
    clearMessage();
    message = "Buffers:\n";
    for (int i = 0; i < (int)tabs.size(); ++i) {
        message +=
            std::to_string(i + 1) + ": " + tabs[i].getFilename() + "\n";
    }
    // Potentially use your renderer to display or just store in 'message'
    // We'll store in 'message' for simplicity:
    refreshRender();
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
    currentTab().moveCursorLeft(t);
    refreshRender();
}
void Editor::moveCursorRight(int t) {
    currentTab().moveCursorRight(t);
    refreshRender();
}
void Editor::moveCursorUp(int t) {
    currentTab().moveCursorUp(t);
    adjustScrolling();
    refreshRender();
}
void Editor::moveCursorDown(int t) {
    currentTab().moveCursorDown(t);
    adjustScrolling();
    refreshRender();
}

// Jump to the beginning of the current line
void Editor::jumpToLineStart() {
    currentTab().jumpToLineStart();
    refreshRender();
}

// Jump to the end of the current line
void Editor::jumpToLineEnd() {
    currentTab().jumpToLineEnd();
    refreshRender();
}

// Go to the first line with the cursor at the beginning
void Editor::goToFirstLine() {
    currentTab().goToFirstLine();
    adjustScrolling();
    refreshRender();
}

// Go to the last line with the cursor at the beginning
void Editor::goToLastLine() {
    currentTab().goToLastLine();
    adjustScrolling();
    refreshRender();
}

void Editor::jumpToLine(int target_line) {
    currentTab().jumpToLine(target_line);
    adjustScrolling();
    refreshRender();
}

// Delete the current line
void Editor::deleteCurrentLine() {
    currentTab().deleteCurrentLine();
    adjustScrolling();
    refreshRender();
}

// Copy the current line
void Editor::copyCurrentLine() {
    // Save the current line to the clipboard
    copied_line = currentTab().getBuffer().getLine(
        currentTab().getCursorY()); 
}

// Paste the copied content
void Editor::pasteContent(int t) {
    currentTab().pasteContent(copied_line, t);
    adjustScrolling();
    refreshRender();
}

// Insert Mode Operations
void Editor::insertCharacter(char c) {
    currentTab().insertCharacter(c);
    refreshRender();
}

void Editor::handleBackspace() {
    currentTab().handleBackspace();
    adjustScrolling();
    refreshRender();
}

void Editor::handleEnter() {
    currentTab().handleEnter();
    adjustScrolling();
    refreshRender();
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
            currentTab().replaceAll(old_str, new_str);
            
            refreshRender();
        } else {
            message = "Insufficient parameter";
        }

    } else {
        message = "Not an editor command: " + command;
    }
    
}

// ===--- Undo/Redo Operations ---===
void Editor::undo() {
    currentTab().undo();
    refreshRender();
}

void Editor::redo() {
    currentTab().redo();
    refreshRender();
}

void Editor::saveFile(const std::string& fname) {
    currentTab().getBuffer().saveToFile(fname);
    // Optionally, display a save confirmation in the status bar
    refreshRender();
}

// Renderer Access
Renderer& Editor::getRenderer() {
    return *renderer;
}
