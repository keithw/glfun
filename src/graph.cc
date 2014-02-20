#include <cmath>
#include <sstream>
#include <locale>
#include <numeric>
#include <limits>
#include <algorithm>

#include "graph.hh"

using namespace std;

Graph::Graph( const unsigned int initial_width, const unsigned int initial_height, const string & title )
  : display_( initial_width, initial_height, title ),
    cairo_( display_.window().size() ),
    pango_( cairo_ ),
    tick_font_( "ACaslon Regular, Normal 30" ),
    label_font_( "ACaslon Regular, Normal 20" ),
    x_tick_labels_(),
    data_points_(),
    x_label_( cairo_, pango_, label_font_, "time (s)" ),
    adjusting_limits_( true ),
    bottom_( 0 ),
    top_( 1 )
{}

void Graph::set_window( const float t, const float logical_width )
{
  while ( (not data_points_.empty()) and (data_points_.front().first < t - logical_width - 1) ) {
    data_points_.pop_front();
  }

  while ( (not x_tick_labels_.empty()) and (x_tick_labels_.front().first < t - logical_width - 1) ) {
    x_tick_labels_.pop_front();
  }
}

static int to_int( const float x )
{
  return static_cast<int>( lrintf( x ) );
}

bool Graph::blocking_draw( const float t, const float logical_width )
{
  /* get the current window size */
  const auto window_size = display_.window().size();

  /* do we need to resize? */
  if ( window_size != cairo_.image().size() ) {
    display_.resize( window_size );
    cairo_ = Cairo( window_size );
  }

  /* start a new image */
  cairo_.mutable_image().clear();

  /* do we need to make a new label? */
  while ( x_tick_labels_.empty() or (x_tick_labels_.back().first < t + 1) ) { /* start when offscreen */
    const int next_label = x_tick_labels_.empty() ? to_int( t ) : x_tick_labels_.back().first + 1;

    /* add commas as appropriate */
    stringstream ss;
    ss.imbue( locale( "" ) );
    ss << fixed << next_label;

    x_tick_labels_.emplace_back( next_label, Pango::Text( cairo_, pango_, tick_font_, ss.str() ) );
  }

  /* draw the labels and vertical grid */
  for ( const auto & x : x_tick_labels_ ) {
    /* position the text in the window */
    const double x_position = window_size.first - (t - x.first) * window_size.first / logical_width;

    x.second.draw_centered_at( cairo_,
			       x_position,
			       window_size.second * 9.0 / 10.0 );

    cairo_set_source_rgba( cairo_, 0, 0, 0.4, 1 );
    cairo_fill( cairo_ );

    /* draw vertical grid line */
    cairo_identity_matrix( cairo_ );
    cairo_set_line_width( cairo_, 2 );
    cairo_move_to( cairo_, x_position, window_size.second * 0.25 / 10.0 );
    cairo_line_to( cairo_, x_position, window_size.second * 8.5 / 10.0 );
    cairo_set_source_rgba( cairo_, 0, 0, 0.4, 0.25 );
    cairo_stroke( cairo_ );
  }

  /* draw the x-axis label */
  x_label_.draw_centered_at( cairo_, window_size.first / 2, window_size.second * 9.6 / 10.0 );
  cairo_set_source_rgba( cairo_, 0, 0, 0.4, 1 );
  cairo_fill( cairo_ );

  /* draw the cairo surface on the OpenGL display */
  display_.draw( cairo_.image() );

  /* draw the data points, including an extension off the screen */
  if ( not data_points_.empty() ) {
    /* adjust bottom and top */
    float data_max = accumulate( data_points_.begin(), data_points_.end(), numeric_limits<float>::min(),
				 [] ( const float x, const pair<float, float> & y ) {
				   return max( x, y.second ); } );
    float data_min = accumulate( data_points_.begin(), data_points_.end(), numeric_limits<float>::max(),
				 [] ( const float x, const pair<float, float> & y ) {
				   return min( x, y.second ); } );

    /* stop adjusting if data is good enough */
    if ( project_height( data_max ) > 0.8 and project_height( data_min ) < 0.2 ) {
      adjusting_limits_ = false;
    }

    /* must adjust if data goes outside the graph */
    if ( project_height( data_max ) > 1.0 or project_height( data_min ) < 0.0 ) {
      adjusting_limits_ = true;
    }

    /* should adjust if data stays too far inside the graph */
    if ( project_height( data_max ) < 0.6 or project_height( data_min ) > 0.4 ) {
      adjusting_limits_ = true;
    }

    if ( adjusting_limits_ ) {
      top_ = top_ * .95 + data_max * .05;
      bottom_ = bottom_ * .95 + data_min * .05;
    }

    data_points_.emplace_back( t + 20, data_points_.back().second );
    display_.draw( 1.0, 0.38, 0.0, 0.75, 5.0, data_points_,
		   [&] ( const pair<float, float> & x ) {
		     return make_pair( window_size.first - (t - x.first) * window_size.first / logical_width,
				       window_size.second * (.825*(1-project_height( x.second ))+.025) );
		   } );
    data_points_.pop_back();
  }

  /* swap buffers to reveal what has been drawn */
  display_.swap();

  /* should we quit? */
  glfwPollEvents();

  if ( display_.window().key_pressed( GLFW_KEY_ESCAPE ) ) {
    return true;
  }

  return false;
}
