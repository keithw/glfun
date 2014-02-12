#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "display.hh"

using namespace std;

void glfun( int argc, char *argv[] );

int main( int argc, char *argv[] )
{
  try {
    glfun( argc, argv );
  } catch ( exception & e ) {
    cerr << "Died on exception: " << e.what() << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void glfun( int argc, char *argv[] )
{
  if ( argc < 1 ) {
    throw runtime_error( "missing argv[ 0 ]" );
  } else if ( argc != 1 ) {
    cerr << "Usage: " << argv[ 0 ] << endl;
    throw runtime_error( "bad command-line arguments" );
  }

  Display display( 640, 480, "OpenGL fun" );

  while ( not display.window().should_close() ) {
    display.repaint();

    glfwPollEvents();

    if ( display.window().key_pressed( GLFW_KEY_ESCAPE ) ) {
      break;
    }
  }
}
