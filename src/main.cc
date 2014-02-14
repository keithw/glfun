#include <cstdlib>
#include <iostream>
#include <stdexcept>

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

  Display display( 640, 480, "OpenGL fun" );

  const auto window_size = display.window().size();
  Image image( window_size.first, window_size.second );

  if ( int( window_size.first ) != cairo_format_stride_for_width( CAIRO_FORMAT_RGB24, window_size.first ) ) {
    throw runtime_error( "Cairo's preferred stride = " + to_string( cairo_format_stride_for_width( CAIRO_FORMAT_RGB24, window_size.first ) ) );
  }

  cairo_surface_t *surface =
    cairo_image_surface_create_for_data( reinterpret_cast<unsigned char *>( &image.mutable_pixels().front() ),
					 CAIRO_FORMAT_RGB24,
					 window_size.first,
					 window_size.second,
					 window_size.first );
  const cairo_status_t result = cairo_surface_status( surface );
  if ( result ) {
    throw runtime_error( string( "cairo error: " ) + cairo_status_to_string( result ) );
  }

  cairo_t *cr =
    cairo_create (surface);

  cairo_select_font_face (cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size (cr, 32.0);
  cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);
  cairo_move_to (cr, 10.0, 50.0);
  cairo_show_text (cr, "Hello, world");
  cairo_destroy (cr);


  while ( not display.window().should_close() ) {
    display.draw( image );

    glfwPollEvents();

    if ( display.window().key_pressed( GLFW_KEY_ESCAPE ) ) {
      break;
    }

    const auto window_size = display.window().size();
    if ( window_size != image.size() ) {
      return;
    }
  }

  cairo_surface_destroy (surface);
}
