#include "gl_objects.hh"

#include <iostream>

using namespace std;

Window::Window( const unsigned int width, const unsigned int height, const string & title )
  : window_()
{
  glfwDefaultWindowHints();

  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );

  glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );

  window_ = glfwCreateWindow( width, height, title.c_str(), nullptr, nullptr );
  if ( not window_ ) {
    throw runtime_error( "could not create window" );
  }
}

void Window::make_context_current( void )
{
  glfwMakeContextCurrent( window_ );
}

bool Window::should_close( void ) const
{
  return glfwWindowShouldClose( window_ );
}

void Window::swap_buffers( void )
{
  return glfwSwapBuffers( window_ );
}

Window::~Window()
{
  glfwHideWindow( window_ );
  glfwDestroyWindow( window_ );
}
