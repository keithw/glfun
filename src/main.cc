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
  Pango::Font caslon( "ACaslon Regular, Normal 30" );
  Pango::Font caslon_small( "ACaslon Regular, Normal 20" );

  double right_edge = 0;

  deque<pair<long int,Pango::Text>> labels;

  deque<pair<float, float>> data_points;

  random_device rd;
  uniform_real_distribution<> dist( 0.8, 1.25 );

  pango.set_font( caslon_small );
  Pango::Text seconds_label( cairo, pango, "time (s)" );
  pango.set_font( caslon );

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
      const long int next_label = labels.empty() ? highest_label - 1 : labels.back().first + 1;

      stringstream ss;
      ss.imbue( locale( "" ) );
      ss << fixed << next_label;// << " µs";

      labels.emplace_back( next_label, Pango::Text( cairo, pango, ss.str() ) );
    }

    /* draw the labels */
    for ( const auto & x : labels ) {
      /* position the text in the window */
      const double x_position = window_size.first - (right_edge - x.first) * window_size.first / 5.0;

      x.second.draw_centered_at( cairo,
				 x_position,
				 window_size.second * 9.0 / 10.0 );

      cairo_set_source_rgba( cairo, 0, 0, 0.4, 1 );
      cairo_fill( cairo );

      /* draw grid */
      cairo_identity_matrix( cairo );
      cairo_set_line_width( cairo, 2 );
      cairo_move_to( cairo, x_position, window_size.second * 0.25 / 10.0 );
      cairo_line_to( cairo, x_position, window_size.second * 8.5 / 10.0 );
      cairo_set_source_rgba( cairo, 0, 0, 0.4, 0.25 );
      cairo_stroke( cairo );
    }

    /* draw the "seconds" label */
    seconds_label.draw_centered_at( cairo,
				    window_size.first / 2,
				    window_size.second * 9.6 / 10.0 );
    cairo_set_source_rgba( cairo, 0, 0, 0.4, 1 );
    cairo_fill( cairo );

    /* draw the "optimal" point */
    cairo_identity_matrix( cairo );
    cairo_set_line_width( cairo, 2 );
    cairo_move_to( cairo, 0, window_size.second / 2.0 );
    cairo_line_to( cairo, window_size.first, window_size.second / 2.0 );
    cairo_set_source_rgba( cairo, 0.4, 0, 0, 0.25 );
    cairo_stroke( cairo );

    /* draw the cairo surface on the OpenGL display */
    display.draw( cairo.image() );

    /* draw some straight lines on top */

    /* should we delete a data point? */
    while ( (!data_points.empty()) and (data_points.front().first < highest_label - 8) ) {
      data_points.pop_front();
    }

    /* do we need a new data point? */
    while ( data_points.empty() or (data_points.back().first < right_edge - 0.05) ) {
      double smoother = data_points.empty() ? 0.5 : data_points.back().second;
      data_points.emplace_back( right_edge, dist( rd ) * smoother );
      if ( data_points.back().second < 0 or data_points.back().second > 1 ) {
	data_points.back().second = 0.5;
      }
    }

    data_points.emplace_back( right_edge + 20, data_points.back().second );

    /* draw the data points */
    display.draw( 1.0, 0.38, 0.0, 0.75, 5.0, data_points,
		  [&] ( const pair<float, float> & x ) {
		    return make_pair( window_size.first - (right_edge - x.first) * window_size.first / 5.0,
				      window_size.second * x.second );
		  } );

    data_points.pop_back();

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
