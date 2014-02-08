#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>

#include "gl_objects.hh"

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

  glfwSetErrorCallback( []( const int, const char * const description )
			{ throw runtime_error( description ); } );

  GLFWContext glfw_context;

  Window window( 640, 480, "OpenGL fun" );
  window.make_context_current();

  glewExperimental = GL_TRUE;
  glewInit();

  VertexBuffer vbo;

  while ( not window.should_close() ) {
    window.swap_buffers();
    glfwPollEvents();
  }
}
