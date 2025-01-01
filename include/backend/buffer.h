// include/backend/buffer.h

#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

/**
 * @brief The Buffer class manages the text data of the editor.
 * 
 * It handles loading from and saving to files, as well as text manipulations
 * such as inserting and deleting characters or lines.
 */
class Buffer {
private:
    // Vector of strings where each string represents a line in the editor
    std::vector<std::string> lines;

public:
    /**
     * @brief Constructs a Buffer object with an initial empty line.
     */
    Buffer();

    /**
     * @brief Loads the content of a file into the buffer.
     * 
     * @param filename The name of the file to load.
     * @return true if the file was loaded successfully, false otherwise.
     */
    bool loadFromFile(const std::string& filename);

    /**
     * @brief Saves the current buffer content to a file.
     * 
     * @param filename The name of the file to save to.
     * @return true if the file was saved successfully, false otherwise.
     */
    bool saveToFile(const std::string& filename) const;

    /**
     * @brief Adds a new line at the end of the buffer.
     * 
     * @param line The content of the line to add.
     */
    void addLine(const std::string& line);

    /**
     * @brief Retrieves the content of a specific line.
     * 
     * @param index The zero-based index of the line.
     * @return The content of the line. Returns an empty string if the index is invalid.
     */
    const std::string& getLine(int index) const;

    std::string& getLine(int index);

    /**
     * @brief Returns the total number of lines in the buffer.
     * 
     * @return The number of lines.
     */
    int getLineCount() const;

    /**
     * @brief Inserts a new line at a specified index.
     * 
     * @param index The zero-based index where the line should be inserted.
     * @param line The content of the line to insert.
     */
    void insertLine(int index, const std::string& line);

    /**
     * @brief Deletes a line at a specified index.
     * 
     * @param index The zero-based index of the line to delete.
     */
    void deleteLine(int index);

    /**
     * @brief Inserts a character into a specific line at a given position.
     * 
     * @param line The zero-based line number.
     * @param pos The zero-based position within the line.
     * @param c The character to insert.
     */
    void insertChar(int line, int pos, char c);

    /**
     * @brief Deletes a character from a specific line at a given position.
     * 
     * @param line The zero-based line number.
     * @param pos The zero-based position within the line.
     */
    void deleteChar(int line, int pos);

    /**
     * @brief Splits a line into two at a specified position.
     * 
     * The text after the position is moved to a new line below.
     * 
     * @param line The zero-based line number to split.
     * @param pos The zero-based position within the line to split at.
     */
    void splitLine(int line, int pos);

    /**
     * @brief Merges the current line with the line below at the specified position.
     * 
     * @param line The zero-based line number to merge with the next line.
     * @param pos The zero-based position where the merge occurs.
     */
    void mergeLines(int line, int pos);

    /**
     * @brief Replaces all occurrences of a substring in a specific line with a new substring.
     * 
     * @param line_number The zero-based line number where replacements should occur.
     * @param old_str The substring to be replaced.
     * @param new_str The substring to replace with.
     */
    void replaceAll(int line_number, const std::string& old_str, const std::string& new_str);

    /**
     * @brief Retrieves all lines in the buffer.
     * 
     * @return A constant reference to the vector of lines.
     */
    const std::vector<std::string>& getLines() const;
};

#endif // BUFFER_H
