#include "BaseWindow.hpp"

#include <imgui.h>
#include "imgui_impl_sdl_gl3.h"
#include <GL/gl3w.h>
#include <SDL2/SDL.h>

#include <stdio.h>
#include <thread>
#include <chrono>

using namespace std;

BaseWindow* BaseWindow::instance = nullptr;

BaseWindow* BaseWindow::getInstance()
{
    return instance;
}

void BaseWindow::run( int width, int height, BaseWindow* window, const char* title )
{ 
    instance = window;
    instance->setup( width, height, title );
    instance->init();
    instance->run();
}

void BaseWindow::setup( int width, int height, const char* title )
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        exit(-1);
    }

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);

    window = SDL_CreateWindow(title,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_OPENGL);

    glContext = SDL_GL_CreateContext(window);
    gl3wInit();

    // Setup ImGui binding
    ImGui_ImplSdlGL3_Init(window);

    SDL_GL_GetDrawableSize(window, &windowWidth, &windowHeight );
}

void BaseWindow::setFPS( int fps )
{
    FPS = fps;
    frameTime = std::chrono::microseconds( 1000000 / FPS );
}

void BaseWindow::run()
{
    ImVec4 clear_color = ImColor(114, 144, 154);

    // disable vsync
    SDL_GL_SetSwapInterval( 0 );

    setFPS(60);

    lastTime = Clock::now();

    // Main loop
    for(;;)
    {
        // prioritize processing all events before rendering
        for( SDL_Event event; SDL_PollEvent(&event); )
        {
            if (event.type == SDL_QUIT) goto done;

            ImGuiIO& io = ImGui::GetIO();
            if( io.WantCaptureKeyboard | io.WantCaptureMouse ) {
                ImGui_ImplSdlGL3_ProcessEvent(&event);
            }

            handleEvent( event );
        }

        ImGui_ImplSdlGL3_NewFrame(window);

        // emulation cycle(s)
        update(); 

        // send draw call commands to Imgui
        gui();

        // Change viewport back to full size for imgui
        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        render();

        // imgui executes draw calls here
        ImGui::Render();

        // screen is updated
        SDL_GL_SwapWindow(window);

        // manual framerate cap
        currentTime = Clock::now();
        deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(
                currentTime - lastTime);

        if( deltaTime < frameTime ) {
            this_thread::sleep_for( chrono::microseconds( frameTime - deltaTime ) );
        }

        lastTime = Clock::now();
    }

done:

    // Cleanup
    ImGui_ImplSdlGL3_Shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
