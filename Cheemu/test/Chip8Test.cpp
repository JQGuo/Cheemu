#include "catch.hpp"
#include "Chip8.hpp"

SCENARIO( "Chip8 tests", "[Chip8]" ) {
    GIVEN( "The Chip8 interpreter" ) {
        C8CPU cpu;

        WHEN( "the state is reset" ) {
            cpu.reset();

            THEN( "I and V registers are zeroed" ) {
                for( auto reg : cpu.VRegister ) {
                    REQUIRE( reg == 0 );
                }

                REQUIRE( cpu.IRegister == 0 );
            }

            THEN( "Delay and Sound timers are zeroed" ) {
                REQUIRE( cpu.delayTimer == 0);
                REQUIRE( cpu.soundTimer == 0);
            }

            THEN( "keyboard is zeroed" ) {
                for( auto key : cpu.keyboard ) {
                    REQUIRE( key == 0 );
                }
            }

            THEN( "stack is zeroed" ) {
                for( auto s : cpu.stack ) {
                    REQUIRE( s == 0 );
                }
            }

            THEN( "pc is 0x200" ) {
                REQUIRE( cpu.pc == 0x200 );
            }

            THEN( "sp is zeroed" ) {
                REQUIRE( cpu.sp == 0 );   
            }

            THEN( "memory should contain fonts before 0x200" ) {
                bool isEmpty = true;
                for( int i = 0; i < 0x200; ++i ) {
                    if( cpu.memory[i] ) {
                        isEmpty = false;
                        break;
                    }
                }

                REQUIRE( !isEmpty );
            }

            THEN( "memory is zeroed at 0x200" ) {
                bool isEmpty = true;
                for( int i = 0x200; i < 0x1000; ++i ) {
                    if( cpu.memory[i] ) {
                        isEmpty = false;
                        break;
                    }
                }

                REQUIRE( isEmpty );
            }
        }
    }
}
