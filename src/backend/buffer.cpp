// src/backend/buffer.cpp

#include "backend/buffer.h"
#include "common/types.h"
#include <cstddef>
#include <fstream>

// Constructor: Initializes the buffer with a single empty line
Buffer::Buffer() : cursor_x(0), cursor_y(0), top_line(0), filename("") {
    lines.emplace_back(""); // at least one line
}

// Loads the buffer content from a file
bool Buffer::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // If the file cannot be opened, return false
        return false;
    }

    lines.clear(); // Clear existing content
    std::string line;
    while (std::getline(file, line)) {
        line.pop_back();
        lines.emplace_back(line);
    }

    // Ensure there is at least one line
    if (lines.empty()) {
        lines.emplace_back("");
    }

    file.close();
    return true;
}

// Saves the buffer content to a file
bool Buffer::saveToFile(const std::string& fname) {
    if (filename.empty() && fname.empty()) {
        // If no filename is specified, return false
        throw std::runtime_error("No filename specified");
    } else if (!fname.empty()) {
        // If a filename is provided, update the buffer's filename
        filename = fname;
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        // If the file cannot be opened for writing, return false
        return false;
    }

    for (const auto& line : lines) {
        file << line << "\n";
    }

    file.close();
    return true;
}

// Adds a new line at the end of the buffer
void Buffer::addLine(const std::string& line) {
    lines.emplace_back(line);
}

// Inserts a new line at a specified index
void Buffer::insertLine(int index, const std::string& line) {
    if (index >= 0 && index <= static_cast<int>(lines.size())) {
        lines.insert(lines.begin() + index, line);
    }
}

// Deletes a line at a specified index
void Buffer::deleteLine(int index) {
    if (index >= 0 && index < static_cast<int>(lines.size())) {
        lines.erase(lines.begin() + index);
        if (lines.empty()) {
            // Ensure there is at least one line
            lines.emplace_back("");
        }
    }
}

// Inserts a character into a specific line at a given position
void Buffer::insertChar(int line, int pos, char c) {
    if (line < 0 || line >= static_cast<int>(lines.size())) {
        return; // Invalid line number
    }
    if (pos < 0 || pos > static_cast<int>(lines[line].size())) {
        return; // Invalid position
    }
    lines[line].insert(lines[line].begin() + pos, c);
}

// Deletes a character from a specific line at a given position
void Buffer::deleteChar(int line, int pos) {
    if (line < 0 || line >= static_cast<int>(lines.size())) {
        return; // Invalid line number
    }
    if (pos < 0 || pos >= static_cast<int>(lines[line].size())) {
        return; // Invalid position
    }
    lines[line].erase(lines[line].begin() + pos);
}

// Splits a line into two at a specified position
void Buffer::splitLine(int line, int pos) {
    if (line < 0 || line >= static_cast<int>(lines.size())) {
        return; // Invalid line number
    }
    if (pos < 0 || pos > static_cast<int>(lines[line].size())) {
        return; // Invalid position
    }

    std::string new_line = lines[line].substr(pos); // Text after the split
    lines[line] = lines[line].substr(0, pos);       // Text before the split
    lines.insert(lines.begin() + line + 1,
                 new_line); // Insert the new line below
}

// Merges the current line with the line below at the specified position
void Buffer::mergeLines(int line, int pos) {
    if (line < 0 || line >= static_cast<int>(lines.size()) - 1) {
        return; // Invalid line number or no line below to merge with
    }
    if (pos < 0 || pos > static_cast<int>(lines[line].size())) {
        return; // Invalid position
    }

    std::string next_line = lines[line + 1];
    lines[line] += next_line; // Append the next line to the current line
    lines.erase(lines.begin() + line + 1); // Remove the next line
}

void Buffer::replaceOneLine(int line, const std::string& old_str, const std::string& new_str) {
    if (line < 0 || line >= static_cast<int>(lines.size())) {
        return; // Invalid line number
    }

    size_t pos = lines[line].find(old_str);
    if (pos != std::string::npos) {
        lines[line].replace(pos, old_str.length(), new_str);
    }
}

void Buffer::replaceAll(const std::string& old_str, const std::string& new_str) {
    Action action;
    action.type = Action::REPLACE;

    // 1) For each line, store the old version before we do the replace
    for (size_t i = 0; i < lines.size(); ++i) {
        std::string& originalLine = lines[i];

        if (originalLine.find(old_str) != std::string::npos) {
            ReplaceLine rl;
            rl.lineNumber = i;
            rl.oldLine = originalLine;
            // We'll fill rc.newLine after the replacement.

            action.replaceLines.push_back(rl);
        }
    }

    // 2) Perform the actual replacement
    for (auto& line : lines) {
        size_t pos = 0;
        while ((pos = line.find(old_str, pos)) != std::string::npos) {
            line.replace(pos, old_str.length(), new_str);
            pos += new_str.length(); // Move past the new substring
        }
    }

    // 3) Now we populate `newLine` in each ReplaceChange from the buffer after
    //    the replacement
    for (auto& rl : action.replaceLines) {
        rl.newLine = lines[rl.lineNumber];
    }

    // 4) If something actually changed, we push it to the undo stack
    //    If no line was changed, action.replaceChanges would be empty
    if (!action.replaceLines.empty()) {
        undo_stack.push(action);
    }
}

// Retrieves the content of a specific line
const std::string& Buffer::getLine(int index) const {
    static const std::string empty_line = "";
    if (index >= 0 && index < static_cast<int>(lines.size())) {
        return lines[index];
    }
    return empty_line;
}

std::string& Buffer::getLine(int index) {
    static std::string empty_line = "";
    if (index >= 0 && index < static_cast<int>(lines.size())) {
        return lines[index];
    }
    return empty_line;
}

// Returns the total number of lines in the buffer
int Buffer::getLineCount() const {
    return static_cast<int>(lines.size());
}

// Retrieves all lines in the buffer
const std::vector<std::string>& Buffer::getLines() const {
    return lines;
}

// ===--- Cursor Movement ---===
void Buffer::moveCursorLeft(int t) {
    cursor_x -= t;
    int min = 0;
    if (cursor_x < min)
        cursor_x = min;
}
void Buffer::moveCursorRight(int t) {
    cursor_x += t;
    int max = static_cast<int>(getLine(cursor_y).size());
    if (cursor_x > max)
        cursor_x = max;
}
void Buffer::moveCursorUp(int t) {
    cursor_y -= t;
    int min = 0;
    if (cursor_y < min)
        cursor_y = min;
    if (cursor_x > static_cast<int>(getLine(cursor_y).size()))
        cursor_x = getLine(cursor_y).size();
}
void Buffer::moveCursorDown(int t) {
    cursor_y += t;
    int max = getLineCount() - 1;
    if (cursor_y > max)
        cursor_y = max;
    if (cursor_x > static_cast<int>(getLine(cursor_y).size()))
        cursor_x = getLine(cursor_y).size();
}
void Buffer::jumpToLineStart() {
    cursor_x = 0;
}
void Buffer::jumpToLineEnd() {
    cursor_x = getLine(cursor_y).size();
}
void Buffer::goToFirstLine() {
    cursor_y = 0;
    cursor_x = 0;
}
void Buffer::goToLastLine() {
    cursor_y = getLineCount() - 1;
    cursor_x = 0;
}
void Buffer::jumpToLine(int target_line) {
    if (target_line < 0)
        target_line = 0;
    else if (target_line >= getLineCount())
        target_line = getLineCount() - 1;
    cursor_y = target_line;
    cursor_x = 0;
}
void Buffer::deleteCurrentLine() {
    // Create an Action for undo
    std::string removed_line = getLine(cursor_y);
    Action action;
    action.type = Action::DELETE_LINE;
    action.line = cursor_y;
    action.pos = 0;             // for a full line delete, pos = 0
    action.text = removed_line; // store the entire line
    undo_stack.push(action);

    deleteLine(cursor_y);

    // If we end up past the last line, adjust cursor
    if (cursor_y >= getLineCount()) {
        cursor_y = getLineCount() - 1;
    }

    cursor_x = 0; // Move cursor to the beginning of the line
}
// void Buffer::copyCurrentLine();

void Buffer::pasteContent(std::string& copied_line, int t) {
    if (copied_line.empty())
        return;

    for (int i = 0; i < t; i++) {
        // Insert new line after the current line
        int insert_index = cursor_y + 1;
        insertLine(insert_index, copied_line);

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
void Buffer::insertCharacter(char c) {
    insertChar(cursor_y, cursor_x, c);
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

void Buffer::handleBackspace() {
    if (cursor_x > 0) {
        char deleted_char = getLine(cursor_y)[cursor_x - 1];
        deleteChar(cursor_y, cursor_x - 1);
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
        int prev_line_length = getLine(cursor_y - 1).size();
        std::string current_line = getLine(cursor_y);
        mergeLines(cursor_y - 1, prev_line_length);
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

void Buffer::handleEnter() {
    splitLine(cursor_y, cursor_x);
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
void Buffer::undo() {
    if (undo_stack.empty())
        return;
    Action action = undo_stack.top();
    undo_stack.pop();

    switch (action.type) {
    case Action::INSERT_CHAR:
        if (action.text == "\n") {
            // We inserted a newline => we splitted a line.
            // Undo means we do the opposite: merge lines.
            mergeLines(action.line, action.pos);

            // Position the cursor so it makes sense after the merge.
            cursor_y = action.line;
            cursor_x = action.pos;
        } else {
            // We inserted a character => undo means we delete it.
            deleteChar(action.line, action.pos);

            // Position the cursor so it makes sense after the deletion.
            cursor_y = action.line;
            cursor_x = action.pos;
        }
        break;

    case Action::DELETE_CHAR:
        if (action.text == "\n") {
            // We removed a newline => line merge
            // The opposite is to split them again
            splitLine(action.line, action.pos);
            // Usually go to the new line
            cursor_y = action.line + 1;
            cursor_x = 0;
        } else {
            // We removed 1 normal char
            insertChar(action.line, action.pos, action.text[0]);
            cursor_y = action.line;
            cursor_x = action.pos + 1;
        }
        break;

    case Action::INSERT_LINE:
        // The opposite is to remove that line
        deleteLine(action.line);
        // Cursor might go to that line if it still exists
        if (action.line >= getLineCount()) {
            cursor_y = getLineCount() - 1;
        } else {
            cursor_y = action.line;
        }
        cursor_x = 0;
        break;

    case Action::DELETE_LINE:
        // The opposite is to re-insert that line
        insertLine(action.line, action.text);
        cursor_y = action.line;
        cursor_x = 0;
        break;

    case Action::REPLACE:
        for (auto& rl : action.replaceLines) {
            lines[rl.lineNumber] = rl.oldLine;
        }
        break;
    }

    // Push the inverse action to redo stack
    redo_stack.push(action);
}

void Buffer::redo() {
    if (redo_stack.empty())
        return;
    Action action = redo_stack.top();
    redo_stack.pop();

    switch (action.type) {
    case Action::INSERT_CHAR:
        if (action.text == "\n") {
            // Re-split lines
            splitLine(action.line, action.pos);
            cursor_y = action.line + 1;
            cursor_x = 0;
        } else {
            insertChar(action.line, action.pos, action.text[0]);
            cursor_y = action.line;
            cursor_x = action.pos + 1;
        }
        break;

    case Action::DELETE_CHAR:
        if (action.text == "\n") {
            // Re-merge lines
            mergeLines(action.line, action.pos);
            cursor_y = action.line;
            cursor_x = action.pos;
        } else {
            deleteChar(action.line, action.pos);
            cursor_y = action.line;
            cursor_x = action.pos;
        }
        break;

    case Action::INSERT_LINE:
        // Re-insert line
        insertLine(action.line, action.text);
        cursor_y = action.line;
        cursor_x = 0;
        break;

    case Action::DELETE_LINE:
        // Re-delete line
        deleteLine(action.line);
        if (cursor_y >= getLineCount()) {
            cursor_y = getLineCount() - 1;
        }
        cursor_x = 0;
        break;

    case Action::REPLACE:
        for (auto& rl : action.replaceLines) {
            lines[rl.lineNumber] = rl.newLine;
        }
        break;
    }

    // Move action to undo stack
    undo_stack.push(action);
}

int Buffer::calculateTopLine(int bottomLine, int COLS, int screen_lines) {
    int topLine = bottomLine;
    int occupy = (lines[topLine].size() / COLS) + 1;
    while (occupy <= screen_lines) {
        occupy += (lines[--topLine].size() / COLS) + 1;
    }
    return ++topLine;
}

void Buffer::ensureCursorWithinBounds() {
    // Ensure cursor_y is within [0, lines.size() - 1]
    if (cursor_y < 0)
        cursor_y = 0;
    if (cursor_y >= static_cast<int>(lines.size())) {
        cursor_y = static_cast<int>(lines.size()) - 1;
    }

    // Ensure cursor_x is within [0, line_length]
    int line_len = (cursor_y >= 0 && cursor_y < (int)lines.size())
                       ? lines[cursor_y].size()
                       : 0;
    if (cursor_x < 0)
        cursor_x = 0;
    if (cursor_x > line_len)
        cursor_x = line_len;
}
