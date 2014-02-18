#include <cairo.h>

#include <stdexcept>

#include "cairo_objects.hh"

using namespace std;

int CairoContext::stride_pixels_for_width( const unsigned int width )
{
  const int stride_bytes = cairo_format_stride_for_width( CAIRO_FORMAT_ARGB32, width );

  if ( stride_bytes % sizeof( Pixel ) ) {
    throw runtime_error( "Cairo requested stride that was not even multiple of pixel size" );
  } else if ( stride_bytes < int( sizeof( Pixel ) * width ) ) {
    throw runtime_error( "Cairo does not support width " + to_string( width ) );
  }

  return stride_bytes / sizeof( Pixel );
}

CairoContext::CairoContext( const std::pair<unsigned int, unsigned int> size )
  : image_( size.first,
	    size.second,
	    stride_pixels_for_width( size.first ) ),
    surface_( image_ ),
    context_( surface_ )
{
  check_error();
}

CairoContext::Surface::Surface( Image & image )
  : surface( cairo_image_surface_create_for_data( image.raw_pixels(),
						  CAIRO_FORMAT_ARGB32,
						  image.size().first,
						  image.size().second,
						  image.stride_bytes() ) )
{
  check_error();
}

CairoContext::Surface::~Surface()
{
  cairo_surface_destroy( surface );
}

CairoContext::Context::Context( Surface & surface )
  : context( cairo_create( surface.surface ) )
{
  check_error();
}

CairoContext::Context::~Context()
{
  cairo_destroy( context );
}

void CairoContext::Surface::check_error( void )
{
  const cairo_status_t surface_result = cairo_surface_status( surface );
  if ( surface_result ) {
    throw runtime_error( string( "cairo surface error: " ) + cairo_status_to_string( surface_result ) );
  }
}

void CairoContext::Context::check_error( void )
{
  const cairo_status_t context_result = cairo_status( context );
  if ( context_result ) {
    throw runtime_error( string( "cairo context error: " ) + cairo_status_to_string( context_result ) );
  }
}

void CairoContext::check_error( void )
{
  context_.check_error();
  surface_.check_error();
}
