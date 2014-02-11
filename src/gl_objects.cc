#include "gl_objects.hh"

#include <iostream>
#include <stdexcept>

using namespace std;

GLFWContext::GLFWContext()
{
  glfwSetErrorCallback( error_callback );

  glfwInit();
}

void GLFWContext::error_callback( const int, const char * const description )
{
  throw runtime_error( description );
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
  glfwWindowHint( GLFW_SAMPLES, 4 );
  glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );

  window_ = glfwCreateWindow( width, height, title.c_str(), nullptr, nullptr );
  if ( not window_ ) {
    throw runtime_error( "could not create window" );
  }
}

void Window::make_context_current( const bool initialize_extensions )
{
  glfwMakeContextCurrent( window_ );

  glCheck( "after MakeContextCurrent" );

  if ( initialize_extensions ) {
    glewExperimental = GL_TRUE;
    glewInit();
    glCheck( "after initializing GLEW", true );
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

std::pair<int, int> Window::size( void ) const
{
  int width, height;
  glfwGetFramebufferSize( window_, &width, &height );
  return make_pair( width, height );
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

VertexBufferObject::~VertexBufferObject()
{
  glDeleteBuffers( 1, &num_ );
}

VertexArrayObject::VertexArrayObject()
  : num_()
{
  glGenVertexArrays( 1, &num_ );
}

VertexArrayObject::~VertexArrayObject()
{
  glDeleteVertexArrays( 1, &num_ );
}

void VertexArrayObject::bind( void )
{
  glBindVertexArray( num_ );
}

Texture::Texture( const unsigned int width, const unsigned int height )
  : num_(),
    width_( width ),
    height_( height )
{
  glGenTextures( 1, &num_ );
}

Texture::~Texture()
{
  glDeleteTextures( 1, &num_ );
}

void Texture::bind( void )
{
  glBindTexture( GL_TEXTURE_RECTANGLE, num_ );
  glTexImage2D( GL_TEXTURE_RECTANGLE, 0, GL_RGBA8, width_, height_, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
}

void Texture::load( const std::vector< Pixel > & pixels )
{
  if ( pixels.size() != width_ * height_ ) {
    throw runtime_error( "vector size does not match texture dimensions" );
  }

  glTexSubImage2D( GL_TEXTURE_RECTANGLE, 0, 0, 0, width_, height_,
		   GL_RGBA, GL_UNSIGNED_BYTE, &pixels.front() );
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

void Program::link( void )
{
  glLinkProgram( num_ );
}

void Program::use( void )
{
  glUseProgram( num_ );
}

GLint Program::attribute_location( const std::string & name )
{
  return glGetAttribLocation( num_, name.c_str() );
}

GLint Program::uniform_location( const std::string & name )
{
  return glGetUniformLocation( num_, name.c_str() );
}

Program::~Program()
{
  glDeleteProgram( num_ );
}

void glCheck( const string & where, const bool expected )
{
  GLenum error = glGetError();

  if ( error != GL_NO_ERROR ) {
    while ( error != GL_NO_ERROR ) {
      if ( not expected ) {
	cerr << "GL error " << where << ": " << gluErrorString( error ) << endl;
      }
      error = glGetError();
    }

    if ( not expected ) {
      throw runtime_error( "GL error " + where );
    }
  }
}
