#ifndef __VIXX_BACKEND_BUFFER_H__
#define __VIXX_BACKEND_BUFFER_H__

#include "common/types.h"
#include <string>
#include <vector>
#include <stack>

class Buffer {

  private:
    std::vector<std::string> lines;

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

    // Accessors
    const std::string& getLine(int index) const;
    std::string& getLine(int index);
    void setLine(int index, const std::string& line);
    int getLineCount() const;
    const std::vector<std::string>& getLines() const;


};

#endif // __VIXX_BACKEND_BUFFER_H__
