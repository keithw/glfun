#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <random>
#include <thread>
#include <chrono>
#include <iomanip>
#include <locale>
#include <sstream>

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
  Pango::Font caslon( "ACaslon Regular, Normal 20" );
  pango.set_font( caslon );

  float scale = 1.0;

  const auto start_time = chrono::high_resolution_clock::now();

  while ( not display.window().should_close() ) {
    scale *= 1.0005;

    cairo.mutable_image().clear();

    /* draw line */
    cairo_identity_matrix( cairo );
    cairo_new_path( cairo );
    cairo_set_line_width( cairo, 20 );
    cairo_move_to( cairo, 50, 200 );
    cairo_line_to( cairo, 400, 200 + scale * 10 );
    cairo_line_to( cairo, 800, 200 - scale * 20 );
    cairo_set_source_rgba( cairo, 0.75, 0, 0, 0.75 );
    cairo_stroke( cairo );

    /* draw text */
    auto age = chrono::duration_cast<chrono::milliseconds>( chrono::high_resolution_clock::now() - start_time );

    stringstream ss;
    ss.imbue( locale( "" ) );
    ss << fixed << age.count() << " ms";

    Pango::Text hello( cairo, pango, ss.str() );

    cairo_identity_matrix( cairo );
    cairo_new_path( cairo );
    cairo_scale( cairo, scale, scale );
    cairo_rotate( cairo, sin( 100 * scale - 1 ) / 10 );

    /* center the text in the window */
    const auto window_size = display.window().size();
    Cairo::Extent<true> extent = hello.extent().to_device( cairo );
    double center_x = window_size.first / 2 - extent.x - extent.width / 2;
    double center_y = window_size.second / 2 - extent.y - extent.height / 2;
    cairo_device_to_user( cairo, &center_x, &center_y );
    cairo_translate( cairo, center_x, center_y );

    /* actually draw the text */
    cairo_append_path( cairo, hello );
    cairo_set_source_rgba( cairo, 0, 0, 0, 1 );
    cairo_fill( cairo );

    /* draw the cairo surface on the OpenGL display */
    display.clear();
    display.draw( cairo.image() );

    /* draw some straight lines using OpenGL alone */
    vector<pair<float, float>> points;

    points.emplace_back( 50, 200 );
    points.emplace_back( 400, 200 - scale * 10 );
    points.emplace_back( 800, 200 + scale * 20 );

    display.draw( 0, 0, 1, 0.75, 20,
		  points );

    /* swap buffers to reveal what has been drawn */
    display.swap();

    /* should we quit or resize? */
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
