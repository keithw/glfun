#include "gl_objects.hh"

#include <iostream>
#include <stdexcept>

using namespace std;

GLFWContext::GLFWContext()
{
  glfwSetErrorCallback( []( const int, const char * const description )
			{ throw runtime_error( description ); } );

  glfwInit();
}

GLFWContext::~GLFWContext()
{
  glfwTerminate();
}

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

void Window::make_context_current( const bool initialize_extensions )
{
  glfwMakeContextCurrent( window_ );

  if ( initialize_extensions ) {
    glewExperimental = GL_TRUE;
    glewInit();
  }
}

bool Window::should_close( void ) const
{
  return glfwWindowShouldClose( window_ );
}

void Window::swap_buffers( void )
{
  return glfwSwapBuffers( window_ );
}

bool Window::key_pressed( const int key ) const
{
  return GLFW_PRESS == glfwGetKey( window_, key );
}

Window::~Window()
{
  glfwHideWindow( window_ );
  glfwDestroyWindow( window_ );
}

VertexBufferObject::VertexBufferObject()
  : num_()
{
  glGenBuffers( 1, &num_ );
}

void compile_shader( const GLuint num, const string & source )
{
  const char * source_c_str = source.c_str();
  glShaderSource( num, 1, &source_c_str, nullptr );
  glCompileShader( num );

  /* check if there were log messages */
  GLint log_length;
  glGetShaderiv( num, GL_INFO_LOG_LENGTH, &log_length );

  if ( log_length > 1 ) {
    std::unique_ptr<GLchar> buffer( new GLchar[ log_length ] );
    GLsizei written_length;
    glGetShaderInfoLog( num, log_length, &written_length, buffer.get() );

    if ( written_length + 1 != log_length ) {
      throw std::runtime_error( "GL shader log size mismatch" );
    }

    cerr << "GL shader compilation log: " << string( buffer.get(), log_length ) << endl;
  }

  /* check if it compiled */
  GLint success;
  glGetShaderiv( num, GL_COMPILE_STATUS, &success );

  if ( not success ) {
    throw runtime_error( "GL shader failed to compile" );
  }
}
