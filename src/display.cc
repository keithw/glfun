#define GL_GLEXT_PROTOTYPES

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <system_error>

#include "display.hh"

using namespace std;

static const char * gluErrorStringChar( const GLenum gl_error )
{
  return reinterpret_cast< const char * >( gl_error );
}

static void GLcheck( const string & description )
{
  const GLenum gl_error = glGetError();

  if ( gl_error != GL_NO_ERROR ) {
    throw runtime_error( description + ": " + gluErrorStringChar( gl_error ) );
  }
}

template <class T>
CheckedPointer<T>::CheckedPointer( const string & context, T const pointer, std::function<void(T)> && deleter )
  : checked_pointer_( pointer ), deleter_( deleter )
{
  if ( checked_pointer_ == nullptr ) {
    throw runtime_error( context + " returned null pointer" );
  }
}

XWindow::XWindow( xcb_connection_t * connection,
		  const unsigned int display_width,
		  const unsigned int display_height )
  : connection_( connection ),
    xcb_screen_( "xcb_setup_roots_iterator",
		 xcb_setup_roots_iterator( xcb_get_setup( connection ) ).data ),
    window_( xcb_generate_id( connection ) )
{
  xcb_create_window( connection,
		     XCB_COPY_FROM_PARENT,
		     window_,
		     xcb_screen_->root,
		     0, 0,
		     display_width, display_height,
		     0,
		     XCB_WINDOW_CLASS_INPUT_OUTPUT,
		     xcb_screen_->root_visual,
		     0, nullptr );

  xcb_map_window( connection, window_ );
}

XWindow::~XWindow( void )
{
  xcb_destroy_window( connection_, window_ );
}

static int desired_attributes[] = { GLX_RGBA,
				    GLX_DOUBLEBUFFER, True,
				    GLX_RED_SIZE, 8,
				    GLX_GREEN_SIZE, 8,
				    GLX_BLUE_SIZE, 8,
				    None };

GLContext::GLContext( Display * display, const XWindow & window )
  : display_( display ),
    visual_( "glXChooseVisual",
	     glXChooseVisual( display, 0, desired_attributes ),
	     []( XVisualInfo * x ) { XFree( x ); } ),
    context_( "glXCreateContext",
	      glXCreateContext( display, visual_, nullptr, true ),
	      [&]( GLXContext x ) { glXDestroyContext( display_, x ); } )
{
  if ( not glXMakeCurrent( display, window, context_ ) ) {
    throw runtime_error( "glXMakeCurrent failed" );
  }

  GLcheck( "glXMakeCurrent" );
}

VideoDisplay::VideoDisplay( const unsigned int width, const unsigned int height )
  : width_( width ), height_( height ),
    display_( "XOpenDisplay", XOpenDisplay( nullptr ), []( Display *x ) { XCloseDisplay( x ); } ),
    xcb_connection_( "XGetXCBConnection",
		     ( XSetEventQueueOwner( display_, XCBOwnsEventQueue ),
		       XGetXCBConnection( display_ ) ) ),
    window_( xcb_connection_, width_, height_ ),
    context_( display_, window_ )
{
  /* initialize viewport */
  glLoadIdentity();
  glViewport( 0, 0, width_, height_ );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0, width_, height_, 0, -1, 1 );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  glWindowPos2d( 0, 0 );
  glClear( GL_COLOR_BUFFER_BIT );
  GLcheck( "viewport" );

  xcb_flush( xcb_connection_ );
}

void VideoDisplay::repaint( void )
{
  glPushMatrix();
  glLoadIdentity();
  glTranslatef( 0, 0, 0 );
  glBegin( GL_POLYGON );

  const double xoffset = 0.25; /* MPEG-2 style 4:2:0 subsampling */

  glMultiTexCoord2d( GL_TEXTURE0, 0, 0 );
  glMultiTexCoord2d( GL_TEXTURE1, xoffset, 0 );
  glMultiTexCoord2d( GL_TEXTURE2, xoffset, 0 );
  glVertex2s( 0, 0 );

  glMultiTexCoord2d( GL_TEXTURE0, width_, 0 );
  glMultiTexCoord2d( GL_TEXTURE1, width_/2 + xoffset, 0 );
  glMultiTexCoord2d( GL_TEXTURE2, width_/2 + xoffset, 0 );
  glVertex2s( width_, 0 );

  glMultiTexCoord2d( GL_TEXTURE0, width_, height_ );
  glMultiTexCoord2d( GL_TEXTURE1, width_/2 + xoffset, height_/2 );
  glMultiTexCoord2d( GL_TEXTURE2, width_/2 + xoffset, height_/2 );
  glVertex2s( width_, height_ );

  glMultiTexCoord2d( GL_TEXTURE0, 0, height_ );
  glMultiTexCoord2d( GL_TEXTURE1, xoffset, height_/2 );
  glMultiTexCoord2d( GL_TEXTURE2, xoffset, height_/2 );
  glVertex2s( 0, height_ );

  glEnd();

  glPopMatrix();

  glXSwapBuffers( display_, window_ );

  GLcheck( "glXSwapBuffers" );

  glFinish();
}
