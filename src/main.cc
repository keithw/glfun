#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <random>
#include <thread>
#include <chrono>

#include <pango/pangocairo.h>

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

  float scale = 1.0;

  while ( not display.window().should_close() ) {
    cairo.mutable_image().clear();

    cairo_set_line_width( cairo, 20 );
    cairo_move_to( cairo, 50, 200 );
    cairo_line_to( cairo, 800, 200 + scale * 10 );
    cairo_stroke( cairo );

    PangoLayout *layout = pango_cairo_create_layout( cairo );

    pango_layout_set_text( layout, "Hello, world.", -1 );

    PangoFontDescription *desc = pango_font_description_from_string("Sans Bold 40");

    pango_layout_set_font_description( layout, desc );

    pango_font_description_free( desc );

    cairo_new_path( cairo );
    cairo_identity_matrix( cairo );
    cairo_scale( cairo, scale, scale );
    scale *= 1.001;

    pango_cairo_layout_path( cairo, layout );
    cairo_path_t * text = cairo_copy_path( cairo );
    
    double x1, y1, x2, y2;
    cairo_path_extents( cairo, &x1, &y1, &x2, &y2 );

    cairo_user_to_device( cairo, &x1, &y1 );
    cairo_user_to_device( cairo, &x2, &y2 );

    cairo_new_path( cairo );

    double center_x = 512 - x1 - (x2 - x1) / 2.0;
    double center_y = 384 - y1 - (y2 - y1) / 2.0;

    cairo_device_to_user( cairo, &center_x, &center_y );

    cairo_translate( cairo, center_x, center_y );
    cairo_append_path( cairo, text );
    cairo_path_destroy( text );

    cairo_set_line_width( cairo, 4 );

    cairo_set_source_rgba( cairo, 0, 1, 1, 1 );

    cairo_stroke_preserve( cairo );
    cairo_set_source_rgba( cairo, 1, 0, 0, 1 );
    cairo_fill( cairo );
    cairo_identity_matrix( cairo );

    g_object_unref( layout );

    display.clear();

    display.draw( cairo.image() );

    vector<pair<uint16_t, uint16_t>> points;

    points.emplace_back( 50, 200 );
    points.emplace_back( 900, 200 - scale * 10 );

    display.draw( 0, 0, 1, 0.25, 20,
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
  }
}
