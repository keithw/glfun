#ifndef CAIRO_OBJECTS_HH
#define CAIRO_OBJECTS_HH

#include <cairo.h>

#include "image.hh"

class CairoContext
{
  Image image_;

  struct Surface
  {
    cairo_surface_t * surface;

    Surface( Image & image );
    ~Surface();

    void check_error( void );

    Surface( const Surface & other ) = delete;
    Surface & operator=( const Surface & other ) = delete;
  } surface_;

  struct Context
  {
    cairo_t * context;

    Context( Surface & surface );
    ~Context();

    void check_error( void );

    Context( const Context & other ) = delete;
    Context & operator=( const Context & other ) = delete;
  } context_;

  static int stride_pixels_for_width( const unsigned int width );
  void check_error( void );

public:
  CairoContext( const std::pair<unsigned int, unsigned int> size );

  operator cairo_t * () { return context_.context; }

  Image & mutable_image( void ) { return image_; }
  const Image & image( void ) const { return image_; }
};

#endif /* CAIRO_OBJECTS_HH */
