// src/main.cpp

#include "../include/backend/editor.h"
#include "../include/frontend/input_handler.h"
#include <iostream>

int main(int argc, char* argv[]) {
    Editor editor;

    if (argc > 1) {
        editor.openFile(argv[1]);
    }

    InputHandler input_handler(editor);

    bool running = true;
    while (running) {
        int ch = getch();
        input_handler.handleInput(ch);
    }

    return 0;
}
