#include <cstdlib>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <cmath>
#include <random>

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
  Window window( 800, 800, "OpenGL fun" );
  window.make_context_current( true );

  VertexBufferObject vbo;

  ArrayBuffer::bind( vbo );

  glCheck( "after loading vertices" );

  VertexShader vertex_shader( R"(
      #version 140

      uniform uvec2 window_size;
      in vec2 position;
      out vec2 raw_position;

      void main()
      {
	gl_Position = vec4( 2 * position.x / window_size.x - 1.0,
                            2 * position.y / window_size.y - 1.0, 0.0, 1.0 );
        raw_position = position;
      }
    )" );

  FragmentShader fragment_shader( R"(
      #version 140

      uniform sampler2DRect tex;

      in vec2 raw_position;
      out vec4 outColor;

      void main()
      {
        outColor = texture( tex, raw_position );
      }
    )" );

  Program program;
  program.attach( vertex_shader );
  program.attach( fragment_shader );
  program.link();
  program.use();

  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  glCheck( "after using shader program" );

  VertexArrayObject vao;
  vao.bind();

  glVertexAttribPointer( program.attribute_location( "position" ), 2, GL_FLOAT, GL_FALSE, 0, 0 );
  glEnableVertexAttribArray( program.attribute_location( "position" ) );

  glfwSwapInterval( 1 );

  glCheck( "starting loop" );

  ios_base::sync_with_stdio( false );

  pair<unsigned int, unsigned int> last_size = window.size();

  glViewport( 0, 0, last_size.first, last_size.second );
  glUniform2ui( program.uniform_location( "window_size" ), last_size.first, last_size.second );

  vector<Pixel> myimage( last_size.first * last_size.second );

  random_device rd;
  uniform_int_distribution<uint8_t> pixel_value( 0, 5 );
  for ( unsigned int y = 0; y < last_size.second; y++ ) {
    for ( unsigned int x = 0; x < last_size.first; x++ ) {
      myimage.at( y * last_size.first + x ) = { uint8_t( 255 * y / last_size.second + pixel_value( rd ) ),
						uint8_t( 255 * x / last_size.first + pixel_value( rd ) ),
						uint8_t( 255 * (x * y) / (last_size.first * last_size.second) + pixel_value( rd ) ),
						128 };
    }
  }

  Texture texture( last_size.first, last_size.second );
  texture.bind();

  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  texture.load( myimage );

  vector<pair<float, float>> corners = { { 0, 0 },
					 { 0, last_size.second },
					 { last_size.first, last_size.second },
					 { last_size.first, 0 } };

  ArrayBuffer::load( corners, GL_STATIC_DRAW );

  while ( not window.should_close() ) {
    for ( auto & x : myimage ) {
      x.red = (x.red + 1) % 255;
      x.green = (x.green + 2) % 255;
      x.blue = (x.blue + 3) % 255;
    }
    texture.load( myimage );

    glClear( GL_COLOR_BUFFER_BIT );
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

    window.swap_buffers();
    glfwPollEvents();

    if ( window.key_pressed( GLFW_KEY_ESCAPE ) ) {
      break;
    }

    const pair<unsigned int, unsigned int> current_size = window.size();
    if ( current_size != last_size ) {
      glViewport( 0, 0, current_size.first, current_size.second );
      last_size = current_size;
      glUniform2ui( program.uniform_location( "window_size" ), last_size.first, last_size.second );

      corners = { { 0, 0 },
		  { 0, last_size.second },
		  { last_size.first, last_size.second },
		  { last_size.first, 0 } };

      ArrayBuffer::load( corners, GL_STATIC_DRAW );
    }
  }

  glCheck( "inside loop" );
}
