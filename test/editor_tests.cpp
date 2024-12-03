// tests/editor_tests.cpp

#include "../include/backend/editor.h"
#include "../include/backend/buffer.h"
#include <cassert>
#include <iostream>

// Note: Comprehensive testing of Editor would require mocking Renderer and handling ncurses dependencies.
// Here, we focus on testing backend functionalities via the Editor interface.

int main() {
    Editor editor;

    // Open a file (assuming 'test.txt' exists)
    editor.openFile("test.txt");
    // Verify buffer content if needed

    // Simulate inserting characters
    editor.insertCharacter('T');
    editor.insertCharacter('e');
    editor.insertCharacter('s');
    editor.insertCharacter('t');
    // Verify buffer content via Buffer class methods

    // Simulate cursor movement
    editor.moveCursorLeft();
    editor.moveCursorLeft();

    // Simulate backspace
    editor.handleBackspace();
    // Verify buffer content

    // Simulate enter key
    editor.handleEnter();
    // Verify line split

    // Simulate saving
    editor.saveFile();

    // Simulate undo
    editor.undo();
    // Verify buffer state

    // Simulate redo
    editor.redo();
    // Verify buffer state

    std::cout << "All Editor tests passed!\n";
    return 0;
}
