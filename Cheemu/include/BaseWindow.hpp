#pragma once

#include <imgui.h>
#include <GL/gl3w.h>
#include <SDL2/SDL.h>

#include <chrono>

typedef std::chrono::high_resolution_clock Clock;

class BaseWindow {
protected:
    BaseWindow() {}

    SDL_Window * window;
    SDL_GLContext glContext;
    int windowWidth, windowHeight;

    virtual void init() {};
    virtual void update() {};
    virtual void gui() {};
    virtual void render() {};
    virtual void handleEvent( SDL_Event &event ) {};

    void setFPS( int FPS );

    // fixed frame rate
    int FPS = 60;

    // these times are in microseconds

    std::chrono::microseconds frameTime;
    Clock::time_point currentTime;
    Clock::time_point lastTime;
    std::chrono::microseconds deltaTime;

public:
    static void run( int width, int height, BaseWindow* window, const char * title );

private:    
    void setup( int width, int height, const char* title );
    void run();

    static BaseWindow* instance;
    static BaseWindow* getInstance();
};
