// tests/buffer_tests.cpp

#include "../include/backend/buffer.h"
#include <cassert>
#include <iostream>

int main() {
    Buffer buffer;

    // Test initial state
    assert(buffer.getLineCount() == 1);
    assert(buffer.getLine(0) == "");

    // Test inserting a character
    buffer.insertChar(0, 0, 'H');
    buffer.insertChar(0, 1, 'i');
    assert(buffer.getLine(0) == "Hi");

    // Test deleting a character
    buffer.deleteChar(0, 1);
    assert(buffer.getLine(0) == "H");

    // Test splitting a line
    buffer.insertChar(0, 1, 'i');
    buffer.insertChar(0, 2, '!');
    buffer.splitLine(0, 2);
    assert(buffer.getLineCount() == 2);
    assert(buffer.getLine(0) == "Hi");
    assert(buffer.getLine(1) == "!");

    // Test replacing text
    buffer.replaceAll(0, "Hi", "Hello");
    assert(buffer.getLine(0) == "Hello");

    std::cout << "All Buffer tests passed!\n";
    return 0;
}
