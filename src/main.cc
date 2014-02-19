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
  Cairo cairo( display.window().size() );

  Pango pango( cairo );

  float scale = 1.0;

  while ( not display.window().should_close() ) {
    scale *= 1.001;

    cairo.mutable_image().clear();

    /*
    cairo_identity_matrix( cairo );

    cairo_set_line_width( cairo, 20 );
    cairo_move_to( cairo, 50, 200 );
    cairo_line_to( cairo, 800, 200 + scale * 10 );
    cairo_stroke( cairo );
    */

    cairo_identity_matrix( cairo );

    cairo_new_path( cairo );

    Pango::Text hello( cairo, pango, "Unaware of grid.", "Sans Bold 40" );

    cairo_scale( cairo, scale, scale );

    cairo_new_path( cairo );

    Cairo::Extent<true> extent = hello.extent().to_device( cairo );

    const auto window_size = display.window().size();
    double center_x = window_size.first / 2 - extent.x - extent.width / 2;
    double center_y = window_size.second / 2 - extent.y - extent.height / 2 - window_size.second / 4;

    cairo_device_to_user( cairo, &center_x, &center_y );

    cairo_translate( cairo, center_x, center_y );

    Pango::Text hello2( cairo, pango, "Aware of grid.", "Sans Bold 40" );

    double motion_x = 0;
    double motion_y = window_size.second / 2;

    cairo_device_to_user( cairo, &motion_x, &motion_y );

    cairo_translate( cairo, center_x + motion_x, center_y + motion_y );

    cairo_append_path( cairo, hello );

    cairo_set_line_width( cairo, 4 );
    cairo_set_source_rgba( cairo, 0, 1, 1, 1 );
    cairo_stroke_preserve( cairo );

    cairo_set_source_rgba( cairo, 1, 0, 0, 1 );
    cairo_fill( cairo );

    display.clear();
    display.draw( cairo.image() );

    /*
    vector<pair<float, float>> points;

    points.emplace_back( 50, 200 );
    points.emplace_back( 900, 200 - scale * 10 );

    display.draw( 0, 0, 1, 0.25, 20,
		  points );
    */

    display.swap();

    glfwPollEvents();

    if ( display.window().key_pressed( GLFW_KEY_ESCAPE ) ) {
      break;
    }

    if ( window_size != cairo.image().size() ) {
      display.resize( window_size );
      cairo = Cairo( window_size );
    }
  }
}
