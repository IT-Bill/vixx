// include/frontend/input_handler.h

#ifndef __VIXX_FRONTEND_INPUT_HANDLER_H__
#define __VIXX_FRONTEND_INPUT_HANDLER_H__

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

#endif // __VIXX_FRONTEND_INPUT_HANDLER_H__
