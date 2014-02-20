#ifndef GRAPH_HH
#define GRAPH_HH

#include "display.hh"
#include "cairo_objects.hh"

class Graph
{
  Display display_;
  Cairo cairo_;
  Pango pango_;

  Pango::Font tick_font_;
  Pango::Font label_font_;

  std::deque<std::pair<int, Pango::Text>> x_tick_labels_;
  std::deque<std::pair<float, float>> data_points_;

  Pango::Text x_label_;

public:
  Graph( const unsigned int initial_width, const unsigned int initial_height, const std::string & title );

  void set_window( const float t, const float logical_width );
  void add_data_point( const float t, const float y ) { data_points_.emplace_back( t, y ); }
  bool blocking_draw( const float t, const float logical_width );
};

#endif /* GRAPH_HH */
