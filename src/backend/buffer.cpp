// src/backend/buffer.cpp

#include "backend/buffer.h"

// Constructor: Initializes the buffer with a single empty line
Buffer::Buffer() {
    lines.emplace_back("");
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
bool Buffer::saveToFile(const std::string& filename) const {
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

    std::string new_line = lines[line].substr(pos);    // Text after the split
    lines[line] = lines[line].substr(0, pos);          // Text before the split
    lines.insert(lines.begin() + line + 1, new_line);  // Insert the new line below
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
    lines[line] += next_line;      // Append the next line to the current line
    lines.erase(lines.begin() + line + 1); // Remove the next line
}

// Replaces all occurrences of a substring in a specific line with a new substring
void Buffer::replaceAll(int line_number, const std::string& old_str, const std::string& new_str) {
    if (line_number < 0 || line_number >= static_cast<int>(lines.size())) {
        return; // Invalid line number
    }

    std::string& line = lines[line_number];
    size_t pos = 0;
    while ((pos = line.find(old_str, pos)) != std::string::npos) {
        line.replace(pos, old_str.length(), new_str);
        pos += new_str.length(); // Move past the new substring
    }
}

// Retrieves all lines in the buffer
const std::vector<std::string>& Buffer::getLines() const {
    return lines;
}
