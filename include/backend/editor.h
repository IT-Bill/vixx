// include/backend/editor.h

#ifndef __VIXX_BACKEND_EDITOR_H__
#define __VIXX_BACKEND_EDITOR_H__

#include "backend/tab.h"
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

    // Tab Management
    void openTab(const std::string& fname = "");
    void closeCurrentTab();
    void switchToNextTab();
    void switchToPreviousTab();
    void switchToTab(int index);
    void listTabs();
    
    // Current Tab Access
    Tab& currentTab();
    const Tab& currentTab() const;

    // Mode Management
    Mode getMode() const;
    void switchMode(Mode new_mode);

    // Multi-file management
    void openFile(const std::string &fname);   // :e <fname>
    void switchBuffer(int index);              // :buffer <n>
    void listBuffers();                        // :ls

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
    void refreshRender();

    // Message Handling
    void setMessage(const std::string& msg);
    void clearMessage();

  private:
    std::vector<Tab> tabs;
    int current_tab_index;
    
    Mode mode;

    std::string number_buffer; // To record digitally-guided commands
    std::string copied_line;
    std::string message;

    Renderer* renderer; // Pointer to Renderer instance
    // InputHandler can be managed separately

    
};

#endif // __VIXX_BACKEND_EDITOR_H__
