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
  Window window( 800, 800, "OpenGL fun" );
  window.make_context_current( true );

  vector<pair<float, float>> vertices = { { 0.0,  0.0 },
					  { 0.1, -0.3 },
					  { 0.2, -0.2 },
					  { 0.3, -0.4 },
					  { 0.4, -0.2 },
					  { 0.4,  0.2 },
					  { 0.5,  0.2 },
					  { 0.5, -0.2 } };

  VertexBufferObject vbo;

  ArrayBuffer::bind( vbo );

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
        outColor = vec4( 1.0, 0.2, 1.0, 0.5 );
      }
    )" );

  Program program;
  program.attach( vertex_shader );
  program.attach( fragment_shader );
  program.link();
  program.use();

  glEnable( GL_BLEND );
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glCheck( "after using shader program" );

  VertexArrayObject vao;
  vao.bind();

  glVertexAttribPointer( program.attribute_location( "position" ), 2, GL_FLOAT, GL_FALSE, 0, 0 );
  glEnableVertexAttribArray( program.attribute_location( "position" ) );

  glfwSwapInterval( 1 );

  glCheck( "starting loop" );

  ios_base::sync_with_stdio( false );

  pair<int, int> last_size = window.size();

  const float width = 0.04;

  while ( not window.should_close() ) {
    for ( auto & x : vertices ) {
            x = make_pair( x.first * cos( .0001 ) - x.second * sin( .0001 ),
			   x.first * sin( .0001 ) + x.second * cos( .0001 ) );
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
    }
  }
}
