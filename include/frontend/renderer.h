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

    void render(const Buffer& buffer, int cursor_x, int cursor_y, Mode mode, const std::string& filename);
    void displayStatusBar(const std::string& mode, const std::string& filename);
    void displayCommandLine(const std::string& command);
    void clearCommandLine();

private:
    bool colors_initialized;
};

#endif // RENDERER_H
