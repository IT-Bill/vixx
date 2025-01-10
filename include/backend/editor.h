// include/backend/editor.h

#ifndef EDITOR_H
#define EDITOR_H

#include "backend/buffer.h"
#include "common/types.h"
#include <ncurses.h>
#include <string>
#include <vector>

class Renderer;     // Forward declaration
class InputHandler; // Forward declaration

class Editor {
  public:
    Editor();
    ~Editor();

    void initialize();
    void shutdown();
    void adjustScrolling();

    void refresh_render();
    void clear_message();

    // Mode Management
    Mode getMode() const;
    void switchMode(Mode new_mode);

    // Multi-file management
    void openFile(const std::string &fname);   // :e <fname>
    void switchBuffer(int index);              // :buffer <n>
    void listBuffers();                        // :ls

    // Current buffer convenience
    Buffer &currentBuffer();
    const Buffer &currentBuffer() const;

    std::string& getNumberBuffer();
    void appendNumberBuffer(const char ch);
    void clearNumberBuffer();

    // Cursor Movement
    void moveCursorLeft(int t);
    void moveCursorRight(int t);
    void moveCursorUp(int t);
    void moveCursorDown(int t);
    void jumpToLineStart();
    void jumpToLineEnd();
    void goToFirstLine();
    void goToLastLine();
    void jumpToLine(int target_line);
    void deleteCurrentLine();
    void copyCurrentLine();
    void pasteContent(int t);

    // Insert Mode Operations
    void insertCharacter(char c);
    void handleBackspace();
    void handleEnter();

    // Command Execution
    void executeCommand(const std::string& command);
    void executeOpenFileCommand(const std::string& fname);

    // Undo/Redo Operations
    void undo();
    void redo();

    // File Operations
    void saveFile(const std::string& fname = "");

    // Renderer Access
    Renderer& getRenderer();

  private:
    std::vector<Buffer> buffers;
    int current_buffer_index;
    
    Mode mode;

    int window_start_line;     // Current window start line
    int window_height;         // Current window height

    std::string number_buffer; // To record digitally-guided commands
    std::string copied_line;
    std::string message;

    Renderer* renderer; // Pointer to Renderer instance
    // InputHandler can be managed separately

    
};

#endif // EDITOR_H
