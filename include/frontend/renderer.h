// include/frontend/renderer.h

#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <vector>
#include "common/types.h"
#include "../backend/buffer.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void initialize();
    void shutdown();

    void render(const Buffer& buffer, int cursor_x, int cursor_y, int top_line, Mode mode, const std::string& filename, const std::string& message);
    void displayStatusBar(const std::string& mode, const std::string& filename, const std::string& message);
    void displayCommandLine(const std::string& command);
    void clearCommandLine();
    int getScreenHeight() const;

    void color_on(int order);
    void color_off(int order);

private:
    bool colors_initialized;
};

#endif // RENDERER_H
