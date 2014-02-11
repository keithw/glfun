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

  vector<pair<float, float>> vertices = { { 200, 200 },
					  { 300, 250 },
					  { 400, 400 },
					  { 400, 700 } };

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

  Texture texture( 1024, 768 );
  texture.bind();

  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  vector< Pixel > myimage( 1024 * 768 );

  random_device rd;
  uniform_int_distribution<> pixel_value( 0, 255 );
  for ( auto & x : myimage ) {
    x.red = pixel_value( rd );
    x.green = pixel_value( rd );
    x.blue = pixel_value( rd );
    x.alpha = pixel_value( rd );
  }

  texture.load( myimage );

  glCheck( "starting loop" );

  ios_base::sync_with_stdio( false );

  pair<int, int> last_size = window.size();

  glViewport( 0, 0, last_size.first, last_size.second );
  glUniform2ui( program.uniform_location( "window_size" ), last_size.first, last_size.second );

  const float width = 20;

  while ( not window.should_close() ) {
    for ( auto & x : vertices ) {
      x = make_pair( (x.first - last_size.first/2) * cos( .01 ) - (x.second - last_size.second/2) * sin( .01 ) + last_size.first/2,
		     (x.first - last_size.first/2) * sin( .01 ) + (x.second - last_size.second/2) * cos( .01 ) + last_size.second/2);
    }

    vector<pair<float, float>> triangles;

    for ( auto it = vertices.begin(); it < vertices.end() - 1; it++ ) {
      const auto & start = *it;
      const auto & end = *(it + 1);

      double rise = (end.second - start.second);
      double run  = (end.first - start.first);

      const double scale_factor = sqrt( rise * rise + run * run );

      rise /= scale_factor;
      run /= scale_factor;

      rise *= width;
      run *= width;

      triangles.emplace_back( start.first - rise, start.second + run );
      triangles.emplace_back( start.first + rise, start.second - run );
      triangles.emplace_back( end.first + rise, end.second - run );

      triangles.emplace_back( start.first - rise, start.second + run );
      triangles.emplace_back( end.first - rise, end.second + run );
      triangles.emplace_back( end.first + rise, end.second - run );

      if ( it < vertices.end() - 2 ) {
	const auto & next = *(it + 2);

	double new_rise = (next.second - end.second);
	double new_run = (next.first - end.first);

	const double new_scale_factor = sqrt( new_rise * new_rise + new_run * new_run );

	new_rise /= new_scale_factor;
	new_run /= new_scale_factor;

	new_rise *= width;
	new_run *= width;

	int run_sign = run >= 0 ? 1 : -1;
	int new_run_sign = new_run >= 0 ? 1 : -1;

	if ( run_sign * new_rise < new_run_sign * rise ) {
	  triangles.emplace_back( end.first - rise, end.second + run );
	  triangles.emplace_back( end.first, end.second );
	  triangles.emplace_back( end.first - new_rise, end.second + new_run );
	} else {
	  triangles.emplace_back( end.first + rise, end.second - run );
	  triangles.emplace_back( end.first, end.second );
	  triangles.emplace_back( end.first + new_rise, end.second - new_run );
	}
      }
    }

    ArrayBuffer::load( triangles, GL_STREAM_DRAW );

    glClear( GL_COLOR_BUFFER_BIT );
    glDrawArrays( GL_TRIANGLES, 0, triangles.size() );

    window.swap_buffers();
    glfwPollEvents();

    if ( window.key_pressed( GLFW_KEY_ESCAPE ) ) {
      break;
    }

    const pair<int, int> current_size = window.size();
    if ( current_size != last_size ) {
      glViewport( 0, 0, current_size.first, current_size.second );
      last_size = current_size;
      glUniform2ui( program.uniform_location( "window_size" ), last_size.first, last_size.second );
    }
  }

  glCheck( "inside loop" );
}
