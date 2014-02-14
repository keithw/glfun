#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <cassert>
#include <random>
#include <thread>
#include <chrono>

#include <cairo.h>

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

  Display display( 1024, 768, "OpenGL fun" );

  const auto window_size = display.window().size();
  int stride = cairo_format_stride_for_width( CAIRO_FORMAT_RGB24, window_size.first );
  if ( stride % sizeof( Pixel ) ) {
    throw runtime_error( "Cairo requested stride that was not even multiple of pixel size" );
  }

  if ( stride < int( sizeof( Pixel ) * window_size.first ) ) {
    throw runtime_error( "Cairo does not support width " + to_string( window_size.first ) );    
  }

  Image image( window_size.first, window_size.second, stride / sizeof( Pixel ) );
  assert( stride == int( image.stride_bytes() ) );

  cairo_surface_t *surface =
    cairo_image_surface_create_for_data( image.raw_pixels(),
					 CAIRO_FORMAT_ARGB32,
					 window_size.first,
					 window_size.second,
					 image.stride_bytes() );
  const cairo_status_t result = cairo_surface_status( surface );
  if ( result ) {
    throw runtime_error( string( "cairo error: " ) + cairo_status_to_string( result ) );
  }

  cairo_t *cr =
    cairo_create (surface);

  random_device rd;
  uniform_real_distribution<float> cols( 0, window_size.first );
  uniform_real_distribution<float> rows( 0, window_size.second );

  cairo_set_antialias( cr, CAIRO_ANTIALIAS_NONE );

  while ( not display.window().should_close() ) {
    image.clear();

    cairo_set_line_width (cr, 1);
    cairo_set_source_rgba (cr, 1, 0, 0, 1);

    float col = cols( rd ), row = rows( rd );

    for ( unsigned int i = 0; i < 1000; i++ ) {
      cairo_move_to( cr, col, row );

      float new_col = int( cols( rd ) );
      cairo_line_to (cr, new_col, row );
      col = new_col;

      float new_row = int( rows( rd ) );
      cairo_line_to (cr, col, new_row );
      row = new_row;
    }

    cairo_stroke (cr);

    display.draw( image );

    glfwPollEvents();

    if ( display.window().key_pressed( GLFW_KEY_ESCAPE ) ) {
      break;
    }

    const auto window_size = display.window().size();
    if ( window_size != image.size() ) {
      return;
    }

    // this_thread::sleep_for( chrono::seconds( 1 ) );
  }

  cairo_destroy (cr);
  cairo_surface_destroy (surface);
}
