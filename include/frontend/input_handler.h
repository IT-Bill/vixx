// include/frontend/input_handler.h

#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <string>

class Editor; // Forward declaration

class InputHandler {
  public:
    InputHandler(Editor& editor);
    ~InputHandler();

    void handleInput(int ch);

  private:
    Editor& editor_ref;
    std::string command_buffer;

    void handleNormalMode(int ch);
    void handleInsertMode(int ch);
    void handleCommandMode(int ch);
    int getNumberBufferOrDefaultOne();
};

#endif // INPUT_HANDLER_H
