// include/common/types.h

#ifndef TYPES_H
#define TYPES_H

#include <string>

// Enumeration for editor modes
enum class Mode {
    NORMAL,
    INSERT,
    COMMAND
};

// Structure to represent an action for undo/redo
struct Action {
    enum Type { INSERT, DELETE, REPLACE } type;
    int line;
    int pos;
    std::string text;
};

#endif // TYPES_H
