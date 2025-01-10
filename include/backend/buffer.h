#ifndef __VIXX_BACKEND_BUFFER_H__
#define __VIXX_BACKEND_BUFFER_H__

#include "common/types.h"
#include <string>
#include <vector>
#include <stack>

class Buffer {

  private:
    std::vector<std::string> lines;
    // Put these new members inside the Buffer:
    int cursor_x;
    int cursor_y;
    int top_line;

    std::stack<Action> undo_stack;
    std::stack<Action> redo_stack;

    std::string filename;

  public:
    // Constructor
    Buffer();

    // File operations
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& fname);

    // Basic line/char manipulation
    void addLine(const std::string& line);
    void insertLine(int index, const std::string& line);
    void deleteLine(int index);
    void insertChar(int line, int pos, char c);
    void deleteChar(int line, int pos);
    void splitLine(int line, int pos);
    void mergeLines(int line, int pos);
    void replaceOneLine(int line, const std::string& old_str, const std::string& new_str);
    void replaceAll(const std::string& old_str, const std::string& new_str);

    // Accessors
    const std::string& getLine(int index) const;
    std::string& getLine(int index);
    int getLineCount() const;
    const std::vector<std::string>& getLines() const;

    // New getters/setters
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

    // Replace Mode Operations

    // Undo/Redo Operations
    void undo();
    void redo();

    void setFilename(const std::string& fname) { filename = fname; }
    const std::string& getFilename() const { return filename; }

    // Scrolling logic can also be put here if you wish:
    int calculateTopLine(int bottomLine, int COLS, int screen_lines);

    // Additional convenience
    void ensureCursorWithinBounds();
};

#endif // __VIXX_BACKEND_BUFFER_H__
