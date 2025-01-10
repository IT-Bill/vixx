// src/backend/tab.h

#ifndef __VIXX_BACKEND_TAB_H__
#define __VIXX_BACKEND_TAB_H__

#include "backend/buffer.h"
#include <string>

class Tab {
  public:
    Tab();
    Tab(const std::string& fname);
    ~Tab();

    Buffer& getBuffer() { return buffer; }
    const Buffer& getBuffer() const { return buffer; }

    void setFilename(const std::string& fname) { filename = fname; }
    const std::string& getFilename() const { return filename; }

    bool isModified() const { return modified; }
    void setModified(bool mod) { modified = mod; }

    void replaceAll(const std::string& old_str, const std::string& new_str);

    // Accessors
    int getCursorX() const { return cursor_x; }
    int getCursorY() const { return cursor_y; }
    int getTopLine() const { return top_line; }
    void setCursorX(int x) { cursor_x = x; }
    void setCursorY(int y) { cursor_y = y; }
    void setTopLine(int t) { top_line = t; }

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
    // void copyCurrentLine();
    void pasteContent(std::string& copied_line, int t);

    // Insert Mode Operations
    void insertCharacter(char c);
    void handleBackspace();
    void handleEnter();

    // Undo/Redo Operations
    void undo();
    void redo();

    // Scrolling logic can also be put here if you wish:
    int calculateTopLine(int bottomLine, int COLS, int screen_lines);

    void adjustScrolling();

    // Additional convenience
    void ensureCursorWithinBounds();

  private:
    Buffer buffer;
    int cursor_x;
    int cursor_y;
    int top_line;

    std::stack<Action> undo_stack;
    std::stack<Action> redo_stack;

    std::string filename;

    bool modified;
};


#endif // __VIXX_BACKEND_TAB_H__