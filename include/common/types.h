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
    enum Type {
        INSERT_CHAR,  // single-character (includes '\n' for line-split)
        DELETE_CHAR,  // single-character (includes '\n' for line-merge)
        INSERT_LINE,  // entire line inserted
        DELETE_LINE,  // entire line deleted

        REPLACE
    };
    Type type;
    int line;
    int pos;
    std::string text;
};

#endif // TYPES_H
