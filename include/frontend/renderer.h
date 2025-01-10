// include/frontend/renderer.h

#ifndef RENDERER_H
#define RENDERER_H

#include "backend/buffer.h"
#include "common/types.h"
#include <string>

class Renderer {
  public:
    Renderer();
    ~Renderer();

    void initialize();
    void shutdown();


    void render(const std::vector<Buffer>& buffers, int current_buffer_index,
                     int cursor_x, int cursor_y, int top_line, Mode mode,
                     const std::string& message, const std::string& number_buffer);
                     
    void renderTabBar(const std::vector<Buffer>& buffers, int current_buffer_index);

    void displayStatusBar(const std::string& mode, const std::string& filename,
                          const std::string& message,
                          const std::string& cmd_buf, const std::string& coor);
    void displayCommandLine(const std::string& command);
    void clearCommandLine();
    int getScreenHeight() const;

    void color_on(int order);
    void color_off(int order);

    int getCOLS();

  private:
    bool colors_initialized;
};

#endif // RENDERER_H
