#include <cstdlib>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <cmath>

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

  vector<pair<float, float>> vertices = { { 0.0,  0.5 },
					  { 0.5, -0.5 },
					  {-0.5, -0.5 } };

  VertexBufferObject vbo;

  ArrayBuffer::bind( vbo );
  ArrayBuffer::load( vertices, GL_STREAM_DRAW );

  glCheck( "after loading vertices" );

  VertexShader vertex_shader( R"(
      #version 140

      in vec2 position;

      void main()
      {
	gl_Position = vec4( position, 0.0, 1.0 );
      }
    )" );

  FragmentShader fragment_shader( R"(
      #version 140

      out vec4 outColor;

      void main()
      {
        outColor = vec4( 1.0, 0.2, 1.0, 1.0 );
      }
    )" );

  Program program;
  program.attach( vertex_shader );
  program.attach( fragment_shader );
  program.link();
  program.use();

  glCheck( "after using shader program" );

  VertexArrayObject vao;
  vao.bind();

  glVertexAttribPointer( program.attribute_location( "position" ), 2, GL_FLOAT, GL_FALSE, 0, 0 );
  glEnableVertexAttribArray( program.attribute_location( "position" ) );

  glfwSwapInterval( 1 );

  glCheck( "starting loop" );

  ios_base::sync_with_stdio( false );

  pair<int, int> last_size = window.size();

  while ( not window.should_close() ) {
    glClear( GL_COLOR_BUFFER_BIT );
    glDrawArrays( GL_LINE_LOOP, 0, 3 );

    window.swap_buffers();
    glfwPollEvents();

    if ( window.key_pressed( GLFW_KEY_ESCAPE ) ) {
      break;
    }

    const pair<int, int> current_size = window.size();
    if ( current_size != last_size ) {
      glViewport( 0, 0, current_size.first, current_size.second );
      last_size = current_size;
    }

    for ( auto & x : vertices ) {
      x = make_pair( x.first * cos( .01 ) - x.second * sin( .01 ),
		     x.first * sin( .01 ) + x.second * cos( .01 ) );
    }

    ArrayBuffer::load( vertices, GL_STREAM_DRAW );
  }
}
