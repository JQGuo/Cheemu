#include "BaseWindow.hpp"
#include "EmuWindow.hpp"

#include <stdio.h>

using namespace std;

void usage( char *argv[] )
{
    printf( "Usage: %s path-to-rom\n", argv[0] );
    exit(-1);
}

int main( int argc, char *argv[]) {
    if( argc != 2 ) usage( argv );

    BaseWindow::run( 1280, 640, new EmuWindow( argv[1] ), "Cheemu" );
    return 0;
}
