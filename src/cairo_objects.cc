#include <stdexcept>

#include "cairo_objects.hh"

using namespace std;

int Cairo::stride_pixels_for_width( const unsigned int width )
{
  const int stride_bytes = cairo_format_stride_for_width( CAIRO_FORMAT_ARGB32, width );

  if ( stride_bytes % sizeof( Pixel ) ) {
    throw runtime_error( "Cairo requested stride that was not even multiple of pixel size" );
  } else if ( stride_bytes < int( sizeof( Pixel ) * width ) ) {
    throw runtime_error( "Cairo does not support width " + to_string( width ) );
  }

  return stride_bytes / sizeof( Pixel );
}

Cairo::Cairo( const pair<unsigned int, unsigned int> size )
  : image_( size.first,
	    size.second,
	    stride_pixels_for_width( size.first ) ),
    surface_( image_ ),
    context_( surface_ )
{
  check_error();
}

Cairo::Surface::Surface( Image & image )
  : surface( cairo_image_surface_create_for_data( image.raw_pixels(),
						  CAIRO_FORMAT_ARGB32,
						  image.size().first,
						  image.size().second,
						  image.stride_bytes() ) )
{
  check_error();
}

Cairo::Context::Context( Surface & surface )
  : context( cairo_create( surface.surface.get() ) )
{
  check_error();
}

void Cairo::Surface::check_error( void )
{
  const cairo_status_t surface_result = cairo_surface_status( surface.get() );
  if ( surface_result ) {
    throw runtime_error( string( "cairo surface error: " ) + cairo_status_to_string( surface_result ) );
  }
}

void Cairo::Context::check_error( void )
{
  const cairo_status_t context_result = cairo_status( context.get() );
  if ( context_result ) {
    throw runtime_error( string( "cairo context error: " ) + cairo_status_to_string( context_result ) );
  }
}

void Cairo::check_error( void )
{
  context_.check_error();
  surface_.check_error();
}

Pango::Pango( Cairo & cairo )
  : context_( pango_cairo_create_context( cairo ) ),
    layout_( pango_layout_new( *this ) )
{}

Pango::Font::Font( const string & description )
  : font( pango_font_description_from_string( description.c_str() ) )
{}

void Pango::set_font( const Pango::Font & font )
{
  pango_layout_set_font_description( *this, font );
}

Pango::Text::Text( Cairo & cairo, Pango & pango, const string & text )
  : path_(),
    extent_( { 0, 0, 0, 0 } )
{
  pango_layout_set_text( pango, text.data(), text.size() );

  pango_cairo_layout_path( cairo, pango );

  path_ = unique_ptr<cairo_path_t, Deleter>( cairo_copy_path( cairo ) );

  /* get logical extents */
  PangoRectangle logical;
  pango_layout_get_extents( pango, nullptr, &logical );
  extent_ = { logical.x / double( PANGO_SCALE ),
	      logical.y / double( PANGO_SCALE ),
	      logical.width / double( PANGO_SCALE ),
	      logical.height / double( PANGO_SCALE ) };
}
