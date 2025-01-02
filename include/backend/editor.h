// include/backend/editor.h

#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>
#include <stack>
#include <ncurses.h>
#include "common/types.h"
#include "backend/buffer.h"

class Renderer;       // Forward declaration
class InputHandler;   // Forward declaration

class Editor {
public:
    Editor();
    ~Editor();

    void refresh_render();
    void clear_message();

    // Mode Management
    Mode getMode() const;
    void switchMode(Mode new_mode);

    // Cursor Movement
    void moveCursorLeft();
    void moveCursorRight();
    void moveCursorUp();
    void moveCursorDown();
    void jumpToLineStart();
    void jumpToLineEnd();
    void goToFirstLine();
    void goToLastLine();
    void deleteCurrentLine();
    void copyCurrentLine();
    void pasteContent();

    // Insert Mode Operations
    void insertCharacter(char c);
    void handleBackspace();
    void handleEnter();

    // Command Execution
    void executeCommand(const std::string& command);

    // Undo/Redo Operations
    void undo();
    void redo();

    // File Operations
    void openFile(const std::string& fname);
    void saveFile();

    // Renderer Access
    Renderer& getRenderer();


private:
    Buffer buffer;
    Mode mode;
    int cursor_x;
    int cursor_y;
    int top_line;
    int window_start_line;  // Current window start line
    int window_height;      // Current window height
    std::string copied_line;
    std::string filename;
    std::string message;
    std::stack<Action> undo_stack;
    std::stack<Action> redo_stack;

    Renderer* renderer; // Pointer to Renderer instance
    // InputHandler can be managed separately

    // Helper Methods
    void initialize();
    void shutdown();
    void adjustScrolling();

};

#endif // EDITOR_H
