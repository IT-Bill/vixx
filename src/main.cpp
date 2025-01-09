// src/main.cpp

#include "backend/editor.h"
#include "frontend/input_handler.h"

int main(int argc, char* argv[]) {
    Editor editor;

    // if (argc > 1) {
    //     editor.openFile(argv[1]);
    // }
    editor.openFile(argc > 1 ? argv[1] : "");

    InputHandler input_handler(editor);

    bool running = true;
    while (running) {
        int ch = getch();
        input_handler.handleInput(ch);
    }

    return 0;
}

