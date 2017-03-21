#include "Chip8.hpp"

#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

#include <spdlog/spdlog.h>
#include "LogSink.hpp"

// use to log instructions
#ifdef NO_LOG
#define LOG_DEBUG( fmt, ... )
#define LOG_INFO( fmt, ... )
#else
#define LOG_DEBUG( fmt, ... ) \
    spdlog::get("C8CPU")->debug( "[ PC: {:#x} ] ( Op: {:#x} ) - " fmt, pc, opcode, ##__VA_ARGS__ )
#define LOG_INFO( fmt, ... ) \
    spdlog::get("C8CPU")->info( fmt, ##__VA_ARGS__ )
#endif

using namespace std;

// TODO
// Clean up this code, separate logic from logging?
// Condense logic into threaded code?

void C8CPU::setLogSink( ExampleAppLog& log )
{
    std::vector<spdlog::sink_ptr> sinks;
    auto logSink = std::make_shared<LogSink>( log );
    logSink->set_level( spdlog::level::info );
    sinks.push_back(logSink);

#ifdef LOG_CONSOLE
    auto stdoutSink = spdlog::sinks::stdout_sink_mt::instance();
    auto colorSink = std::make_shared<spdlog::sinks::ansicolor_sink>(stdoutSink);
    sinks.push_back(colorSink);
#endif

    auto C8Logger = std::make_shared<spdlog::logger>("C8CPU", begin(sinks), end(sinks));
    C8Logger->set_pattern("<%L:%n> %v");
    spdlog::register_logger(C8Logger);

    spdlog::set_level( spdlog::level::debug );
}

void C8CPU::executeInstruction()
{ 
    // convert opcode from big to little endian
    opcode = memory[pc] << 8 | memory[pc + 1];

    // decode opcode into the parts we may need
    unsigned short opNNN = opcode & 0x0FFF;
    unsigned short opNN = opcode & 0x00FF;
    unsigned short opN = opcode & 0x000F;
    unsigned short opX = ( opcode & 0x0F00 ) >> 8;
    unsigned short opY = ( opcode & 0x00F0 ) >> 4;

    // check first hex char to branch on
    switch( opcode >> 12 ) {
        case 0x0: {
            if( opcode == 0x00E0 ) {
                // clear screen;
                LOG_DEBUG( "Clear the screen." );
                memset( screen, 0, sizeof( screen ) );
            } else if ( opcode == 0x00EE ) {
                // return from subroutine
                LOG_DEBUG( "Return from subroutine." );
                --sp;
                pc = stack[ sp ];
            } else { 
                // run program RCA 1802 at NNN, not used?
                // TODO
                LOG_DEBUG( "Execute machine language subroutine at {:#x}", opNNN );
                throw runtime_error( "0NNN opcode not implemented." );
            }

            break;
        }
        case 0x1: {
            // 1NNN: jump to address NNN
            LOG_DEBUG( "pc = {:#x}", opNNN );
            pc = opNNN - 2;
            break;
        }
        case 0x2: {
            // 2NNN: call subroutine at NNN
            LOG_DEBUG( "Call subroutine at {:#x}", opNNN );
            stack[ sp ] = pc;
            ++sp;
            pc = opNNN - 2;
            break;
        }
        case 0x3: {
            // 3XNN: skip if VX == NN
            LOG_DEBUG( "skip if V[{:x}] == {:#x}", opX, opNN );
            if ( VRegister[ opX ] == opNN ) {
                pc += 2;
            }
            break;
        }
        case 0x4: {
            // 4XNN: skip if VX != NN
            LOG_DEBUG( "skip if V[{:x}] != {:#x}", opX, opNN );
            if ( VRegister[ opX ] != opNN ) {
                pc += 2;
            }
            break;
        }
        case 0x5: {
            // 5XY0: skip if VX == VY
            LOG_DEBUG( "skip if V[{:x}] == V[{:x}]", opX, opY );
            if ( VRegister[ opX ] == VRegister[ opY ] ) {
                pc += 2;
            }
            break;
        }
        case 0x6: {
            // 6XNN: set VX to NN
            LOG_DEBUG( "V[{:x}] = {:#x}", opX, opNN );
            VRegister[ opX ] = opNN;
            break;
        }
        case 0x7: {
            // 7XNN: add NN to VX
            LOG_DEBUG( "V[{:x}] += {:#x}", opX, opNN );
            VRegister[ opX ] += opNN;
            break;
        }
        case 0x8: {
            switch( opcode & 0x000F ) {
                case 0x0: {
                    // 8XY0: VX = VY
                    LOG_DEBUG( "V[{:x}] = V[{:x}]", opX, opY );
                    VRegister[ opX ] = VRegister[ opY ];
                    break;
                }
                case 0x1: {
                    // 8XY1: VX |= VY
                    LOG_DEBUG( "V[{:x}] |= V[{:x}]", opX, opY );
                    VRegister[ opX ] |= VRegister[ opY ];
                    break;
                }
                case 0x2: {
                    // 8XY2: VX &= VY
                    LOG_DEBUG( "V[{:x}] &= V[{:x}]", opX, opY );
                    VRegister[ opX ] &= VRegister[ opY ];
                    break;
                }
                case 0x3: {
                    // 8XY3: VX ^= VY
                    LOG_DEBUG( "V[{:x}] ^= V[{:x}]", opX, opY );
                    VRegister[ opX ] ^= VRegister[ opY ];
                    break;
                }
                case 0x4: {
                    // 8XY4: VX += VY, VF = carry
                    LOG_DEBUG( "V[{:x}] += V[{:x}]", opX, opY );
                    unsigned short result = VRegister[ opX ] + VRegister[ opY ];
                    if( result > 255 ) VRegister[ 0xF ] = 1;

                    // this is same as % for unsigned values
                    VRegister[ opX ] = result & 0xFF;
                    break;
                }
                case 0x5: {
                    // 8XY5: VX -= VY, VF = !borrow
                    LOG_DEBUG( "V[{:x}] -= V[{:x}]", opX, opY );
                    LOG_DEBUG( "{} - {}", VRegister[opX], VRegister[opY] );
                    VRegister[ 0xF ] = ( VRegister[ opX ] >= VRegister[ opY ] );
                    VRegister[ opX ] -= VRegister[ opY ];
                    break;
                }
                case 0x6: {
                    // 8XY6: VX >>= 1, VF = LSB( VX )
                    LOG_DEBUG( "V[{:x}] >>= 1", opX );
                    VRegister[ 0xF ] = ( VRegister[ opX ] & 0x1 );
                    VRegister[ opX ] >>= 1;
                    break;
                }
                case 0x7: {
                    // 8XY7: VX = VY - VX, VF = !borrow
                    LOG_DEBUG( "V[{:x}] = V[{:x}] - V[{:x}]", opX, opY, opX );
                    VRegister[ 0xF ] = ( VRegister[ opY ] >= VRegister[ opX ] );
                    VRegister[ opX ] = VRegister[ opY ] - VRegister[ opX ];
                    break;
                }
                case 0xE: {
                    // 8XYE: VX <<= 1, VF = MSB( VX )
                    LOG_DEBUG( "V[{:x}] <<= 1", opX );
                    VRegister[ 0xF ] = ( ( VRegister[ opX ] & 0x80 ) >> 7 );
                    VRegister[ opX ] <<= 1;
                    break;
                }
                default: {
                    throw runtime_error( "Invalid opcode." );
                }
            }
            break;
        }
        case 0x9: {
            // 9XY0: skip if VX != VY
            LOG_DEBUG( "skip if V[{:x}] != V[{:x}]", opX, opY );
            if ( VRegister[ opX ] != VRegister[ opY ] ) {
                pc += 2;
            }
            break;
        }
        case 0xA: {
            // ANNN: I = NNN
            LOG_DEBUG( "I = {:#x}", opNNN );
            IRegister = opNNN;
            break;
        }
        case 0xB: {
            // BNNN: jump to address V0 + NNN
            LOG_DEBUG( "pc = V[0] + {:#x}", opNNN );
            pc = VRegister[0] + opNNN - 2;
            break;               
        }
        case 0xC: {
            // CXNN: VX = ( rand() % 255 ) & NN
            LOG_DEBUG( "V[{:x}] = rand() & {:#x}", opX, opNN );
            VRegister[ opX ] = rand() & 0xFF & opNN;
            break;
        }
        case 0xD: {
            // DXYN: draw( VX, VY, N ), draw sprite at ( VX, VY ) that has a width of
            // 8 pixels and a height of N pixels. VF is set if any screen pixels are
            // flipped from set to unset when the sprite is drawn, 0 otherwise
            unsigned char * spriteDataPtr;
            unsigned char spriteData;

            short baseXPos = VRegister[ opX ]; 
            short baseYPos = VRegister[ opY ];

            bool collision = false;

            // for each row
            for( short yOffset = 0; yOffset < opN; ++yOffset ) { 
                // program tried to draw below the screen
                if( baseYPos + yOffset > 31 ) break;

                // get char representing row of sprite
                spriteDataPtr = ( unsigned char * ) &memory[ IRegister + yOffset ];
                spriteData = *spriteDataPtr;

                // sprites have fixed 8 columns
                for( short xOffset = 0; xOffset < 8; ++xOffset ) {
                    // program tried to draw right of the screen
                    if( baseXPos + xOffset > 63 ) break;

                    // check each bit of the char and xor with existing screen pos
                    if( spriteData & 0x80 ) {
                        // check if any existing pixel gets erased as a result
                        if( screen[ baseXPos + xOffset ][ baseYPos + yOffset ] ) {
                            collision = true;
                        }

                        screen[ baseXPos + xOffset ][ baseYPos + yOffset ] ^= 1;
                    }

                    spriteData <<= 1;
                }
            }

            // set VF flag
            if( collision ) {
                VRegister[0xF] = 1;
            } else {
                VRegister[0xF] = 0;
            }

            LOG_DEBUG( "draw( {}, {} ), w=8 "
                    "h={}, carry={}",
                    baseXPos, baseYPos, opN, VRegister[0xF] );

            /*
            LOG_DEBUG( "\n\n" );
            for( int y = 0; y < 32; ++y ) {
                for( int x = 0; x < 64; ++x ) {
                    if( screen[x][y] ) {
                        LOG_DEBUG("*");
                    } else {
                        LOG_DEBUG(" ");
                    }
                }
                LOG_DEBUG("\n");
            }
            LOG_DEBUG( "\n\n" );
            */

            break;
        }
        case 0xE: {
            if( ( opcode & 0x00FF ) == 0x9E ) {
                // EX9E: skip if hex key in VX is pressed
                if( keyboard[ VRegister[ opX ] ] ) {
                    LOG_DEBUG( "skip if V[{:x}] = {:x} pressed => yes", opX, VRegister[ opX ] );
                    pc += 2;
                } else {
                    LOG_DEBUG( "skip if V[{:x}] = {:x} pressed => no", opX, VRegister[ opX ] );
                }
            } else if ( ( opcode & 0x00FF ) == 0xA1 ) {
                // EX9E: skip if hex key in VX is not pressed
                if( !keyboard[ VRegister[ opX ] ] ) {
                    LOG_DEBUG( "skip if V[{:x}] = %x not pressed => yes", opX, VRegister[ opX ] );
                    pc += 2;
                } else {
                    LOG_DEBUG( "skip if V[{:x}] = %x not pressed => no", opX, VRegister[ opX ] );
                }
            } else {
                throw runtime_error( "Invalid opcode." );
            }

            break;
        }
        case 0xF: {
            switch( opcode & 0x00FF ) {
                case 0x07: {
                    // FX07: VX = delayTimer
                    LOG_DEBUG( "V[{:x}] = delayTimer = {}", opX, delayTimer );
                    VRegister[ opX ] = delayTimer;
                    break;
                }
                case 0x0A: {
                    // FX0A: Wait for keypress, VX = keypress
                    // TODO: Wait for key release if already pressed?

                    // use the last key pressed if there are any keys pressed
                    if( numPressed ) {
                        LOG_DEBUG( "V[{:x}] = keypress = {:#x}", opX, lastKey );
                        VRegister[ opX ] = lastKey;
                    } else {
                        LOG_DEBUG( "V[{:x}] = keypress, no input given", opX, lastKey );
                        pc -= 2;
                    }

                    break;
                }
                case 0x15: {
                    // FX15: delayTimer = VX
                    LOG_DEBUG( "delayTimer = V[{:x}]", opX );
                    delayTimer = VRegister[ opX ];
                    break;
                }
                case 0x18: {
                    // FX18: soundTimer = VX
                    LOG_DEBUG( "delayTimer = V[{:x}]", opX );
                    soundTimer = VRegister[ opX ];
                    break;
                }
                case 0x1E: {
                    // FX1E: I += VX
                    LOG_DEBUG( "I += V[{:x}]", opX );
                    IRegister += VRegister[ opX ];
                    break;
                }
                case 0x29: {
                    // FX29: I = address for sprite of digit in VX
                    LOG_DEBUG( "I = &sprite of digit in V[{:x}]", opX );
                    IRegister = 8 * VRegister[ opX ];
                    break;
                }
                case 0x33: {
                    // FX33: BCD of VX in I, I+1, I+2 (hundreds, tens, ones), I remains unchanged
                    unsigned short decimal = VRegister[ opX ];

                    memory[ IRegister + 2 ] = decimal % 10;
                    memory[ IRegister + 1 ] = ( decimal / 10 ) % 10;
                    memory[ IRegister ] = decimal / 100;

                    LOG_DEBUG( "Store BCD of V[{:x}] = {}=> I = {}, I+1 = {}, I+2 = {}",
                            opX,
                            decimal,
                            memory[ IRegister ],
                            memory[ IRegister + 1 ],
                            memory[ IRegister + 2 ]);
                    break;
                }
                case 0x55: {
                    // FX55: Store V0 through VX starting from I, I remains unchanged
                    LOG_DEBUG( "Store V[0] - V[{:x}] at I = {:#x}", opX, IRegister );

                    for( unsigned short i = 0; i <= opX; ++i ) {
                        memory[ IRegister + i ] = VRegister[ i ];
                    }

                    break;
                }
                case 0x65: {
                    // FX65: Load V0 through VX starting from I, I remains unchanged
                    LOG_DEBUG( "Load into V[0] - V[{:x}] from I = {:#x}", opX, IRegister );

                    for( unsigned short i = 0; i <= opX; ++i ) {
                        VRegister[ i ] = memory[ IRegister + i ];
                    }

                    break;
                }
                default: {
                    throw runtime_error( "Invalid opcode." );
                }
            }

            break;
        }
        default: {
            throw runtime_error( "Invalid opcode." );
        }
    }

    // advance program counter
    pc += 2;
}

void C8CPU::loadROM( const char * path )
{
    romPath = path;

    // reset the state of the interpreter
    reset();

    // load the ROM starting at address 0x200
    ifstream rom( path, ios::binary );

    if( !rom.good() ) throw runtime_error( "Invalid ROM path." );

    LOG_INFO( "Loading ROM from {}...", path );

    streambuf *rawbuf = rom.rdbuf();
    rawbuf->sgetn( ( char * ) &memory[0x200], rawbuf->in_avail() );

    LOG_INFO( "ROM loaded." );
}

void C8CPU::reset()
{
    LOG_INFO( "Resetting Chip8 state." );

    // set state to 0 for easier debugging
    memset( screen, 0, sizeof( screen ) );
    memset( keyboard, 0, sizeof( keyboard ) );
    memset( memory, 0, sizeof( memory ) );
    memset( stack, 0, sizeof( stack ) );
    memset( VRegister, 0, sizeof( VRegister ) );

    IRegister = 0;
    opcode = 0;
    sp = 0;
    
    delayTimer = 0;
    soundTimer = 0;

    // assume that ROM is loaded starting at 0x200
    pc = 0x200;

    // fill in provided sprite data into interpreter reserved area of memory [ 0x0, 0x1FF ]
    // sprite "0"
    memory[0] = 0xF0;
    memory[1] = 0x90;
    memory[2] = 0x90;
    memory[3] = 0x90;
    memory[4] = 0xF0;

    // sprite "1"
    memory[8] = 0x20;
    memory[9] = 0x60;
    memory[10] = 0x20;
    memory[11] = 0x20;
    memory[12] = 0x70;

    // sprite "2"
    memory[16] = 0xF0;
    memory[17] = 0x10;
    memory[18] = 0xF0;
    memory[19] = 0x80;
    memory[20] = 0xF0;

    // sprite "3"
    memory[24] = 0xF0;
    memory[25] = 0x10;
    memory[26] = 0xF0;
    memory[27] = 0x10;
    memory[28] = 0xF0;

    // sprite "4"
    memory[32] = 0x90;
    memory[33] = 0x90;
    memory[34] = 0xF0;
    memory[35] = 0x10;
    memory[36] = 0x10;

    // sprite "5"
    memory[40] = 0xF0;
    memory[41] = 0x80;
    memory[42] = 0xF0;
    memory[43] = 0x10;
    memory[44] = 0xF0;

    // sprite "6"
    memory[48] = 0xF0;
    memory[49] = 0x80;
    memory[50] = 0xF0;
    memory[51] = 0x90;
    memory[52] = 0xF0;

    // sprite "7"
    memory[56] = 0xF0;
    memory[57] = 0x10;
    memory[58] = 0x20;
    memory[59] = 0x40;
    memory[60] = 0x40;

    // sprite "8"
    memory[64] = 0xF0;
    memory[65] = 0x90;
    memory[66] = 0xF0;
    memory[67] = 0x90;
    memory[68] = 0xF0;

    // sprite "9"
    memory[72] = 0xF0;
    memory[73] = 0x90;
    memory[74] = 0xF0;
    memory[75] = 0x10;
    memory[76] = 0xF0;

    // sprite "A"
    memory[80] = 0xF0;
    memory[81] = 0x90;
    memory[82] = 0xF0;
    memory[83] = 0x90;
    memory[84] = 0x90;

    // sprite "B"
    memory[88] = 0xE0;
    memory[89] = 0x90;
    memory[90] = 0xE0;
    memory[91] = 0x90;
    memory[92] = 0xE0;

    // sprite "C"
    memory[96] = 0xF0;
    memory[97] = 0x80;
    memory[98] = 0x80;
    memory[99] = 0x80;
    memory[100] = 0xF0;

    // sprite "D"
    memory[104] = 0xE0;
    memory[105] = 0x90;
    memory[106] = 0x90;
    memory[107] = 0x90;
    memory[108] = 0xE0;

    // sprite "E"
    memory[112] = 0xF0;
    memory[113] = 0x80;
    memory[114] = 0xF0;
    memory[115] = 0x80;
    memory[116] = 0xF0;

    // sprite "F"
    memory[120] = 0xF0;
    memory[121] = 0x80;
    memory[122] = 0xF0;
    memory[123] = 0x80;
    memory[124] = 0x80;
}

