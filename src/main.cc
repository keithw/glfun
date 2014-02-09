#include <cstdlib>
#include <iostream>
#include <vector>
#include <stdexcept>

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

  GLFWContext glfw_context;

  Window window( 640, 480, "OpenGL fun" );
  window.make_context_current( true );

  vector< pair< float, float > > vertices;

  vertices.emplace_back(  0.0,  0.5 );
  vertices.emplace_back(  0.5, -0.5 );
  vertices.emplace_back( -0.5, -0.5 );

  VertexBufferObject vbo;

  vbo.bind<ArrayBuffer>();
  ArrayBuffer::load( vertices, GL_STREAM_DRAW );

  VertexShader shader( R"(
      #version 140

      in vec2 position;

      void main()
      {
	gl_Position = vec4( position, 0.0, 1.0 );
      }
    )" );

  while ( not window.should_close() ) {
    window.swap_buffers();
    glfwPollEvents();

    if ( window.key_pressed( GLFW_KEY_ESCAPE ) ) {
      break;
    }
  }
}
