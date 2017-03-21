#include "EmuWindow.hpp"
#include "MemEdit.hpp"
#include <spdlog/spdlog.h>

#include <limits.h>

EmuWindow::EmuWindow() {}

EmuWindow::EmuWindow( const char * romPath )
{ 
    chip8.setLogSink( appLog );
    chip8.loadROM( romPath );
    spdlog::get("C8CPU")->info("Emulation started.");
}

EmuWindow::~EmuWindow()
{
    delete am;
}

void EmuWindow::init()
{
    am = new AudioManager();
    am->loadSamples();

    keymap[SDLK_1] = 0x1;
    keymap[SDLK_2] = 0x2;
    keymap[SDLK_3] = 0x3;
    keymap[SDLK_q] = 0x4;
    keymap[SDLK_w] = 0x5;
    keymap[SDLK_e] = 0x6;
    keymap[SDLK_a] = 0x7;
    keymap[SDLK_s] = 0x8;
    keymap[SDLK_d] = 0x9;

    keymap[SDLK_z] = 0xA;
    keymap[SDLK_x] = 0x0;
    keymap[SDLK_c] = 0xB;

    keymap[SDLK_4] = 0xC;
    keymap[SDLK_r] = 0xD;
    keymap[SDLK_f] = 0xE;
    keymap[SDLK_v] = 0xF;

    prog.initProgram();
    prog.addShader( "shaders/vert.vs", GL_VERTEX_SHADER );
    prog.addShader( "shaders/frag.fs", GL_FRAGMENT_SHADER );
    prog.linkProgram();

    float vertex_data[64][32][6][2] = { 0.0f };

    // create grid of 64x32 pixels for the screen
    // made up of 2 tris, x and y per vertex
    for( int x = 0; x < 64; ++x ) {
        for( int y = 0; y < 32; ++y ) {
            vertex_data[x][y][0][0] = 2.0f * x / 64.0f - 1.0f;
            vertex_data[x][y][0][1] = 2.0f * ( 32.0f - y ) / 32.0f - 1.0f;
            vertex_data[x][y][1][0] = 2.0f * x / 64.0f - 1.0f;
            vertex_data[x][y][1][1] = 2.0f * ( 32.0f - y - 1.0f ) / 32.0f - 1.0f;
            vertex_data[x][y][2][0] = 2.0f * ( x + 1.0f ) / 64.0f - 1.0f;
            vertex_data[x][y][2][1] = 2.0f * ( 32.0f - y - 1.0f ) / 32.0f - 1.0f;

            vertex_data[x][y][3][0] = 2.0f * x / 64.0f - 1.0f;
            vertex_data[x][y][3][1] = 2.0f * ( 32.0f - y ) / 32.0f - 1.0f;
            vertex_data[x][y][4][0] = 2.0f * ( x + 1.0f ) / 64.0f - 1.0f;
            vertex_data[x][y][4][1] = 2.0f * ( 32.0f - y - 1.0f ) / 32.0f - 1.0f;
            vertex_data[x][y][5][0] = 2.0f * ( x + 1.0f ) / 64.0f - 1.0f;
            vertex_data[x][y][5][1] = 2.0f * ( 32.0f - y ) / 32.0f - 1.0f;
        }
    }

    float colour_data[64][32][6] = { 0 };

    /*
    // single pixel
    for( int k = 0; k < 6; ++k ) {
        colour_data[63][31][k] = 1.0f;
    }
    */

    /*
    // checkerboard
    for( int i = 0; i < 64; ++i ) {
        for( int j = 0; j < 32; ++j ) {
            for( int k = 0; k < 6; ++k ) {
                if( (i + j) % 2 ) {
                    colour_data[i][j][k] = 1.0f;
                } else {
                    colour_data[i][j][k] = 0.0f;
                }
            }
        }
    }
    */

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // transfer data
    glGenBuffers( 1, &vertex_data_buffer );
    glBindBuffer( GL_ARRAY_BUFFER, vertex_data_buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertex_data ), vertex_data, GL_STATIC_DRAW );

    glGenBuffers( 1, &vertex_colour_buffer );
    glBindBuffer( GL_ARRAY_BUFFER, vertex_colour_buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof( colour_data ), colour_data, GL_STREAM_DRAW );

    // map attributes
    prog.enable();

    glBindBuffer( GL_ARRAY_BUFFER, vertex_data_buffer );
    glEnableVertexAttribArray( 0 ); 
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0 );

    glBindBuffer( GL_ARRAY_BUFFER, vertex_colour_buffer );
    glEnableVertexAttribArray( 1 ); 
    glVertexAttribPointer( 1, 1, GL_FLOAT, GL_FALSE, 0, (void*) 0 );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void EmuWindow::update()
{
    if( !isStopped ) {
        // first decrement timers
        // TODO: these should be updated at 60hz, but are tied to the frame rate instead
        if( chip8.delayTimer ) --chip8.delayTimer;
        if( chip8.soundTimer ) --chip8.soundTimer;

        if( chip8.soundTimer > 0 ) {
            am->play();
        }

        // if playing, perform IPF # of instructions per update
        for( int i = 0; i < IPF; ++i ) chip8.executeInstruction();
    } else if( doStep ) {
        // if stepping, only execute one instruction
        chip8.executeInstruction();
        doStep = false;
    }

    float colour_data[64][32][6] = { 0 };

    // get frame from Chip8's screen
    for( int i = 0; i < 64; ++i ) {
        for( int j = 0; j < 32; ++j ) {
            for( int k = 0; k < 6; ++k ) {
                colour_data[i][j][k] = chip8.screen[i][j];
            }
        }
    }

    glBindBuffer( GL_ARRAY_BUFFER, vertex_colour_buffer );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( colour_data ), colour_data );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void EmuWindow::gui()
{
	static bool showDebugWindow(false);

    // these local variables are needed because ImGui doesn't have min/max checks for InputInt
    static int fps = FPS;
    static int ipf = IPF;
    static char label[20];

    static MemoryEditor mem_editor;

    ImGui::SetNextWindowSize(ImVec2(465, 370), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(50, 0), ImGuiSetCond_FirstUseEver);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Begin("CPU Info", &showDebugWindow );

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::BeginChild("Sub1", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.3f, 0 ), true);

	ImGui::Text("Registers");
	ImGui::Separator();

    for( int i = 0; i <= 0xF; ++i ) {
        sprintf(label, "V[%X] = %#x", i, chip8.VRegister[i]);
        ImGui::Text("%s", label);
	}

    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("Sub2", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.3f, 0 ), true);

	ImGui::Text("Stack");
	ImGui::Separator();

    for( int i = 0; i <= 0xF; ++i ) {
        sprintf(label, "S[%X] = %#x", i, chip8.stack[i]);
        ImGui::Text("%s", label);
	}

    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("Sub3", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.4f, 0 ), true);
    sprintf(label, "PC = %#x", chip8.pc);
    ImGui::Text("%s", label);
    sprintf(label, "SP = %#x", chip8.sp);
    ImGui::Text("%s", label);
    sprintf(label, "I = %#x", chip8.IRegister);
    ImGui::Text("%s", label);

    ImGui::Separator();

    sprintf(label, "DT = %#x", chip8.delayTimer);
    ImGui::Text("%s", label);
    sprintf(label, "ST = %#x", chip8.soundTimer);
    ImGui::Text("%s", label);

    ImGui::Separator();
    if( ImGui::InputInt("FPS", &fps, 10, 1, ImGuiInputTextFlags_EnterReturnsTrue ) ) {
        if( fps >= 30 && fps <= 1000 ) {
            setFPS( fps );
        } else {
            fps = FPS;
        }
    }

    if( ImGui::InputInt("IPF", &ipf, 1, 1, ImGuiInputTextFlags_EnterReturnsTrue ) ) {
        if( ipf > 0 && ipf <= 10 ) {
            IPF = ipf;
        } else {
            ipf = IPF;
        }
    }

    ImGui::Separator();
    sprintf(label, "Instructions/sec = %d", fps * ipf);
    ImGui::Text("%s", label);

    ImGui::Separator();
    ImGui::Spacing();
    if( ImGui::Button("Reset") ) {
        chip8.loadROM( chip8.romPath );
    }
    ImGui::SameLine();
    if( ImGui::Button("Stop") ) {
        spdlog::get("C8CPU")->info("Emulation stopped.");
        spdlog::get("C8CPU")->sinks()[0]->set_level( spdlog::level::debug );
        isStopped = true;
    }

    ImGui::Spacing();
    if( ImGui::Button("Play") ) {
        spdlog::get("C8CPU")->info("Emulation started.");
        spdlog::get("C8CPU")->sinks()[0]->set_level( spdlog::level::info );
        isStopped = false;
    }
    ImGui::SameLine();
    if( ImGui::Button("Step") ) {
        doStep = isStopped;
    }

    ImGui::EndChild();

    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(465, 270), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(50, 370), ImGuiSetCond_FirstUseEver);

    mem_editor.Draw("Memory Editor", chip8.memory, sizeof( chip8.memory ) );

    ImGui::SetNextWindowSize(ImVec2(640, 320), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(640, 320), ImGuiSetCond_FirstUseEver);

    appLog.Draw("Log");
}

void EmuWindow::render()
{
    // render to top right
    glViewport( windowWidth / 2, windowHeight / 2, windowWidth / 2, windowHeight / 2);
	glDrawArrays( GL_TRIANGLES, 0, 64 * 32 * 6 );
}

void EmuWindow::handleEvent( SDL_Event &event )
{
    if( event.type == SDL_KEYDOWN || event.type == SDL_KEYUP ) {
        auto key = event.key.keysym.sym;

        if( key == SDLK_ESCAPE ) {
            event.type = SDL_QUIT;
            SDL_PushEvent( &event );
            return;
        }

        bool isKeyDown = bool( event.type == SDL_KEYDOWN );

        if( keymap.count( key ) ) {
            chip8.keyboard[ keymap[ key ] ] = isKeyDown;
            chip8.lastKey = keymap[ key ];
            chip8.numPressed += 2 * isKeyDown - 1;
        }
    }
}

