#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <random>
#include <thread>
#include <chrono>
#include <iomanip>
#include <locale>
#include <sstream>
#include <deque>
#include <algorithm>

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
  Pango::Font caslon( "ACaslon Regular, Normal 24" );
  pango.set_font( caslon );

  double right_edge = -1;

  deque<pair<long int,Pango::Text>> labels;

  while ( not display.window().should_close() ) {
    right_edge += 1.0 / 240.0;

    const auto window_size = display.window().size();

    cairo.mutable_image().clear();

    const long int highest_label = lrint( floor( right_edge ) ) + 1;

    /* should we delete a label? */
    while ( (!labels.empty()) and (labels.front().first < highest_label - 10) ) {
      labels.pop_front();
    }

    /* do we need a new label? */
    while ( labels.empty() or (labels.back().first != highest_label) ) {
      const long int next_label = labels.empty() ? highest_label : labels.back().first + 1;

      stringstream ss;
      ss.imbue( locale( "" ) );
      ss << fixed << next_label;// << " µs";

      cairo_identity_matrix( cairo );
      cairo_new_path( cairo );
      labels.emplace_back( next_label, Pango::Text( cairo, pango, ss.str() ) );
    }

    /* draw the labels */
    for ( const auto & x : labels ) {
      /* position the text in the window */
      cairo_identity_matrix( cairo );
      cairo_new_path( cairo );
      Cairo::Extent<true> extent = x.second.extent().to_device( cairo );
      double center_x = window_size.first - extent.x - extent.width / 2 - (right_edge - x.first) * window_size.first / 5.0;
      double center_y = window_size.second - extent.y - extent.height;
      cairo_device_to_user( cairo, &center_x, &center_y );
      cairo_translate( cairo, center_x, center_y );
      cairo_append_path( cairo, x.second );

      /* actually draw the text */
      cairo_set_source_rgba( cairo, 0, 0, 0.4, 1 );
      cairo_fill( cairo );
    }

    /* draw the cairo surface on the OpenGL display */
    display.clear();
    display.draw( cairo.image() );

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
