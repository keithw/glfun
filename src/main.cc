#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <random>
#include <thread>
#include <chrono>

#include "display.hh"
#include "cairo_objects.hh"

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

  Display display( 1024, 768, "OpenGL fun" );
  CairoContext cairo( display.window().size() );

  random_device rd;
  uniform_int_distribution<> cols( 0, display.window().size().first );
  uniform_int_distribution<> rows( 0, display.window().size().second );

  while ( not display.window().should_close() ) {
    cairo.mutable_image().clear();

    cairo_set_line_width( cairo, 2 );
    cairo_set_source_rgba( cairo, 1, 0, 0, 0.75 );

    float col = cols( rd ), row = rows( rd );

    cairo_move_to( cairo, col, row );

    for ( unsigned int i = 0; i < 1000; i++ ) {
      float new_col = cols( rd );
      cairo_line_to( cairo, new_col, row );
      col = new_col;

      float new_row = rows( rd );
      cairo_line_to( cairo, col, new_row );
      row = new_row;
    }

    cairo_stroke( cairo );

    display.clear();

    display.draw( cairo.image() );

    vector<pair<uint16_t, uint16_t>> points;
    points.reserve( 10000 );

    for ( unsigned int i = 0; i < 1000; i++ ) {
      points.emplace_back( cols( rd ), rows( rd ) );
    }

    display.draw( 0, 0, 1, 0.25, 1,
		  points );

    display.swap();

    glfwPollEvents();

    if ( display.window().key_pressed( GLFW_KEY_ESCAPE ) ) {
      break;
    }

    const auto window_size = display.window().size();
    if ( window_size != cairo.image().size() ) {
      return;
    }

    //    this_thread::sleep_for( chrono::milliseconds( 250 ) );
  }
}
