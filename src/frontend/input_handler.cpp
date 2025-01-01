// src/frontend/input_handler.cpp

#include "frontend/input_handler.h"
#include "backend/editor.h"
#include "common/types.h"
#include "frontend/renderer.h"
#include <cctype>

// Constructor
InputHandler::InputHandler(Editor& editor) : editor_ref(editor), command_buffer("") {}

// Destructor
InputHandler::~InputHandler() {}

// Handle input based on current mode
void InputHandler::handleInput(int ch) {
    Mode current_mode = editor_ref.getMode();
    switch (current_mode) {
        case Mode::NORMAL:
            handleNormalMode(ch);
            break;
        case Mode::INSERT:
            handleInsertMode(ch);
            break;
        case Mode::COMMAND:
            handleCommandMode(ch);
            break;
    }
}

// Handle inputs in Normal mode
void InputHandler::handleNormalMode(int ch) {
    static int last_char = 0; // To track multi-character commands
    bool done = false;

    switch (last_char) {
        case 'g':
            if (ch == 'g') editor_ref.goToFirstLine();
            done = true; break;
        case 'd':
            if (ch == 'd') editor_ref.deleteCurrentLine();
            done = true; break;
        case 'y':
            if (ch == 'y') editor_ref.copyCurrentLine();
            done = true; break;
        default: break;
    }
    last_char = 0;
    if (done) return;

    switch (ch) {
        case 'i':
            editor_ref.switchMode(Mode::INSERT);
            break;
        case ':':
            editor_ref.switchMode(Mode::COMMAND);
            command_buffer.clear();
            break;
        case 'h': case 260:
            editor_ref.moveCursorLeft();
            break;
        case 'j': case 258:
            editor_ref.moveCursorDown();
            break;
        case 'k': case 259:
            editor_ref.moveCursorUp();
            break;
        case 'l': case 261:
            editor_ref.moveCursorRight();
            break;
        case 'u':
            editor_ref.undo();
            break;
        case 'R':
            editor_ref.redo();
            break;
        case '0':
            editor_ref.jumpToLineStart();
            break;
        case '$':
            editor_ref.jumpToLineEnd();
            break;
        case 'G':
            editor_ref.goToLastLine();
            break;
        case 'p':
            editor_ref.pasteContent();
            break;
        default:
            last_char = ch;
            break;
    }
}

// Handle inputs in Insert mode
void InputHandler::handleInsertMode(int ch) {
    switch (ch) {
        case 27: // ESC key
            editor_ref.switchMode(Mode::NORMAL);
            break;
        case KEY_BACKSPACE:
        case 127:
            editor_ref.handleBackspace();
            break;
        case '\n':
            editor_ref.handleEnter();
            break;
        default:
            if (isprint(ch)) {
                editor_ref.insertCharacter(static_cast<char>(ch));
            }
            break;
    }
}

// Handle inputs in Command mode
void InputHandler::handleCommandMode(int ch) {
    Renderer& renderer = editor_ref.getRenderer();
    if (ch == '\n') {
        editor_ref.executeCommand(command_buffer);
        editor_ref.switchMode(Mode::NORMAL);
    }
    else if (ch == 27) { // ESC key
        editor_ref.switchMode(Mode::NORMAL);
    }
    else if (ch == KEY_BACKSPACE || ch == 127) {
        if (!command_buffer.empty()) {
            command_buffer.pop_back();
            renderer.displayCommandLine(command_buffer);
        }
    }
    else {
        if (isprint(ch)) {
            command_buffer += static_cast<char>(ch);
            renderer.displayCommandLine(command_buffer);
        }
    }
}
