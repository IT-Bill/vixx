// src/backend/tab.cpp

#include "backend/tab.h"
#include "backend/buffer.h"
#include <cstddef>
#include <fstream>

// clang-format off
Tab::Tab()
    : buffer()
    , cursor_x(0)
    , cursor_y(0)
    , top_line(0)
    , filename("")
    , modified(false) {}

Tab::Tab(const std::string& fname)
    : buffer()
    , cursor_x(0)
    , cursor_y(0)
    , top_line(0)
    , filename(fname)
    , modified(false) {
    if (!buffer.loadFromFile(fname)) {
        // Handle file load failure if necessary
    }
}

Tab::~Tab() {}
// clang-format on

void Tab::replaceAll(const std::string& old_str, const std::string& new_str) {
    Action action;
    action.type = Action::REPLACE;

    // 1) For each line, store the old version before we do the replace
    for (size_t i = 0; i < buffer.getLineCount(); ++i) {
        std::string& originalLine = buffer.getLine(i);

        if (originalLine.find(old_str) != std::string::npos) {
            ReplaceLine rl;
            rl.lineNumber = i;
            rl.oldLine = originalLine;
            // We'll fill rc.newLine after the replacement.

            action.replaceLines.push_back(rl);
        }
    }

    // 2) Perform the actual replacement
    for (size_t i = 0; i < buffer.getLineCount(); ++i) {
        buffer.replaceOneLine(i, old_str, new_str);
    }

    // 3) Now we populate `newLine` in each ReplaceChange from the buffer after
    //    the replacement
    for (auto& rl : action.replaceLines) {
        rl.newLine = buffer.getLine(rl.lineNumber);
    }

    // 4) If something actually changed, we push it to the undo stack
    //    If no line was changed, action.replaceChanges would be empty
    if (!action.replaceLines.empty()) {
        undo_stack.push(action);
    }
}

// ===--- Cursor Movement ---===
void Tab::moveCursorLeft(int t) {
    cursor_x -= t;
    int min = 0;
    if (cursor_x < min)
        cursor_x = min;
}
void Tab::moveCursorRight(int t) {
    cursor_x += t;
    int max = static_cast<int>(buffer.getLine(cursor_y).size());
    if (cursor_x > max)
        cursor_x = max;
}
void Tab::moveCursorUp(int t) {
    cursor_y -= t;
    int min = 0;
    if (cursor_y < min)
        cursor_y = min;
    if (cursor_x > static_cast<int>(buffer.getLine(cursor_y).size()))
        cursor_x = buffer.getLine(cursor_y).size();
}
void Tab::moveCursorDown(int t) {
    cursor_y += t;
    int max = buffer.getLineCount() - 1;
    if (cursor_y > max)
        cursor_y = max;
    if (cursor_x > static_cast<int>(buffer.getLine(cursor_y).size()))
        cursor_x = buffer.getLine(cursor_y).size();
}
void Tab::jumpToLineStart() {
    cursor_x = 0;
}
void Tab::jumpToLineEnd() {
    cursor_x = buffer.getLine(cursor_y).size();
}
void Tab::goToFirstLine() {
    cursor_y = 0;
    cursor_x = 0;
}
void Tab::goToLastLine() {
    cursor_y = buffer.getLineCount() - 1;
    cursor_x = 0;
}
void Tab::jumpToLine(int target_line) {
    if (target_line < 0)
        target_line = 0;
    else if (target_line >= buffer.getLineCount())
        target_line = buffer.getLineCount() - 1;
    cursor_y = target_line;
    cursor_x = 0;
}
void Tab::deleteCurrentLine() {
    // Create an Action for undo
    std::string removed_line = buffer.getLine(cursor_y);
    Action action;
    action.type = Action::DELETE_LINE;
    action.line = cursor_y;
    action.pos = 0;             // for a full line delete, pos = 0
    action.text = removed_line; // store the entire line
    undo_stack.push(action);

    buffer.deleteLine(cursor_y);

    // If we end up past the last line, adjust cursor
    if (cursor_y >= buffer.getLineCount()) {
        cursor_y = buffer.getLineCount() - 1;
    }

    cursor_x = 0; // Move cursor to the beginning of the line
}
// void Buffer::copyCurrentLine();

void Tab::pasteContent(std::string& copied_line, int t) {
    if (copied_line.empty())
        return;

    for (int i = 0; i < t; i++) {
        // Insert new line after the current line
        int insert_index = cursor_y + 1;
        buffer.insertLine(insert_index, copied_line);

        // Record action
        Action action;
        action.type = Action::INSERT_LINE;
        action.line = insert_index;
        action.pos = 0;
        action.text = copied_line;
        undo_stack.push(action);

        // Move the cursor downward to the newly inserted line
        cursor_y = insert_index;
        cursor_x = 0;
    }
}

// ===--- Insert Mode Operations ---===
void Tab::insertCharacter(char c) {
    buffer.insertChar(cursor_y, cursor_x, c);
    // Record action for undo
    Action action;
    action.type = Action::INSERT_CHAR;
    action.line = cursor_y;
    action.pos = cursor_x;
    action.text = std::string(1, c);
    undo_stack.push(action);

    // Update cursor position
    cursor_x++;
}

void Tab::handleBackspace() {
    if (cursor_x > 0) {
        char deleted_char = buffer.getLine(cursor_y)[cursor_x - 1];
        buffer.deleteChar(cursor_y, cursor_x - 1);
        // Record action for undo
        Action action;
        action.type = Action::DELETE_CHAR;
        action.line = cursor_y;
        action.pos = cursor_x - 1;
        action.text = std::string(1, deleted_char);
        undo_stack.push(action);
        // Update cursor position
        cursor_x--;
    } else if (cursor_y > 0) {
        // Merge with previous line
        int prev_line_length = buffer.getLine(cursor_y - 1).size();
        std::string current_line = buffer.getLine(cursor_y);
        buffer.mergeLines(cursor_y - 1, prev_line_length);
        // Record action for undo (line merge)
        Action action;
        action.type = Action::DELETE_CHAR;
        action.line = cursor_y - 1;
        action.pos = prev_line_length;
        action.text = "\n"; // Representing line merge
        undo_stack.push(action);
        // Update cursor position
        cursor_y--;
        cursor_x = prev_line_length;
    }
}

void Tab::handleEnter() {
    buffer.splitLine(cursor_y, cursor_x);
    // Record action for undo (line split)
    Action action;
    action.type = Action::INSERT_CHAR;
    action.line = cursor_y;
    action.pos = cursor_x;
    action.text = "\n"; // Representing line split
    undo_stack.push(action);
    // Move to the new line
    cursor_y++;
    cursor_x = 0;
}

// ===--- Undo/Redo Operations ---===
void Tab::undo() {
    if (undo_stack.empty())
        return;
    Action action = undo_stack.top();
    undo_stack.pop();

    switch (action.type) {
    case Action::INSERT_CHAR:
        if (action.text == "\n") {
            // We inserted a newline => we splitted a line.
            // Undo means we do the opposite: merge lines.
            buffer.mergeLines(action.line, action.pos);

            // Position the cursor so it makes sense after the merge.
            cursor_y = action.line;
            cursor_x = action.pos;
        } else {
            // We inserted a character => undo means we delete it.
            buffer.deleteChar(action.line, action.pos);

            // Position the cursor so it makes sense after the deletion.
            cursor_y = action.line;
            cursor_x = action.pos;
        }
        break;

    case Action::DELETE_CHAR:
        if (action.text == "\n") {
            // We removed a newline => line merge
            // The opposite is to split them again
            buffer.splitLine(action.line, action.pos);
            // Usually go to the new line
            cursor_y = action.line + 1;
            cursor_x = 0;
        } else {
            // We removed 1 normal char
            buffer.insertChar(action.line, action.pos, action.text[0]);
            cursor_y = action.line;
            cursor_x = action.pos + 1;
        }
        break;

    case Action::INSERT_LINE:
        // The opposite is to remove that line
        buffer.deleteLine(action.line);
        // Cursor might go to that line if it still exists
        if (action.line >= buffer.getLineCount()) {
            cursor_y = buffer.getLineCount() - 1;
        } else {
            cursor_y = action.line;
        }
        cursor_x = 0;
        break;

    case Action::DELETE_LINE:
        // The opposite is to re-insert that line
        buffer.insertLine(action.line, action.text);
        cursor_y = action.line;
        cursor_x = 0;
        break;

    case Action::REPLACE:
        for (auto& rl : action.replaceLines) {
            buffer.replaceOneLine(rl.lineNumber, rl.newLine, rl.oldLine);
        }
        break;
    }

    // Push the inverse action to redo stack
    redo_stack.push(action);
}

void Tab::redo() {
    if (redo_stack.empty())
        return;
    Action action = redo_stack.top();
    redo_stack.pop();

    switch (action.type) {
    case Action::INSERT_CHAR:
        if (action.text == "\n") {
            // Re-split lines
            buffer.splitLine(action.line, action.pos);
            cursor_y = action.line + 1;
            cursor_x = 0;
        } else {
            buffer.insertChar(action.line, action.pos, action.text[0]);
            cursor_y = action.line;
            cursor_x = action.pos + 1;
        }
        break;

    case Action::DELETE_CHAR:
        if (action.text == "\n") {
            // Re-merge lines
            buffer.mergeLines(action.line, action.pos);
            cursor_y = action.line;
            cursor_x = action.pos;
        } else {
            buffer.deleteChar(action.line, action.pos);
            cursor_y = action.line;
            cursor_x = action.pos;
        }
        break;

    case Action::INSERT_LINE:
        // Re-insert line
        buffer.insertLine(action.line, action.text);
        cursor_y = action.line;
        cursor_x = 0;
        break;

    case Action::DELETE_LINE:
        // Re-delete line
        buffer.deleteLine(action.line);
        if (cursor_y >= buffer.getLineCount()) {
            cursor_y = buffer.getLineCount() - 1;
        }
        cursor_x = 0;
        break;

    case Action::REPLACE:
        for (auto& rl : action.replaceLines) {
            buffer.setLine(rl.lineNumber, rl.newLine);
        }
        break;
    }

    // Move action to undo stack
    undo_stack.push(action);
}

int Tab::calculateTopLine(int bottomLine, int COLS, int screen_lines) {
    int topLine = bottomLine;
    int occupy = (buffer.getLine(topLine).size() / COLS) + 1;
    while (occupy <= screen_lines) {
        occupy += (buffer.getLine(--topLine).size() / COLS) + 1;
    }
    return ++topLine;
}


void Tab::ensureCursorWithinBounds() {
    // Ensure cursor_y is within [0, lines.size() - 1]
    if (cursor_y < 0)
        cursor_y = 0;
    if (cursor_y >= static_cast<int>(buffer.getLineCount())) {
        cursor_y = static_cast<int>(buffer.getLineCount()) - 1;
    }

    // Ensure cursor_x is within [0, line_length]
    int line_len = (cursor_y >= 0 && cursor_y < (int)buffer.getLineCount())
                       ? buffer.getLine(cursor_y).size()
                       : 0;
    if (cursor_x < 0)
        cursor_x = 0;
    if (cursor_x > line_len)
        cursor_x = line_len;
}





