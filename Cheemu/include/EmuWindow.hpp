#pragma once

#include "BaseWindow.hpp"
#include "ShaderProgram.hpp"

#include "Chip8.hpp"
#include "ExampleAppLog.hpp"
#include "AudioManager.hpp"

#include <unordered_map>

class EmuWindow : public BaseWindow {
public:
    EmuWindow();
    EmuWindow( const char * rom );
    ~EmuWindow();

private:
    void init() override;
    void update() override;
    void gui() override;
    void render() override;
    void handleEvent( SDL_Event &event ) override;

    // map keys to inputs Chip8 interpreter expects
    std::unordered_map< int, int > keymap;

    // the Chip8 interpreter
    C8CPU chip8;

    // instructions per frame
    int IPF = 1;
    bool isStopped = false, doStep = false;

    // OpenGL stuff
    ShaderProgram prog;
    GLuint vao, vertex_data_buffer, vertex_colour_buffer;

    // Wrapper for SDL Audio calls
    AudioManager * am;

    // ExampleAppLog needs to be in the class so we can pass reference to C8CPU
    ExampleAppLog appLog;
};
