#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>

#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

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

class GLFWContext
{
public:
  GLFWContext() { glfwInit(); }
  ~GLFWContext() { glfwTerminate(); }
};

void error_callback( const int, const char * const description )
{
  throw runtime_error( description );
}

void glfun( int argc, char *argv[] )
{
  if ( argc < 1 ) {
    throw runtime_error( "missing argv[ 0 ]" );
  } else if ( argc != 1 ) {
    cerr << "Usage: " << argv[ 0 ] << endl;
    throw runtime_error( "bad command-line arguments" );
  }

  glfwSetErrorCallback( error_callback );

  GLFWContext glfw_context;

  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );

  glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );

  GLFWwindow * window = glfwCreateWindow( 640, 480, "OpenGL Fun", nullptr, nullptr );
  if ( window == nullptr ) {
    throw runtime_error( "could not create window" );
  }

  glfwMakeContextCurrent( window );

  glewExperimental = GL_TRUE;
  glewInit();

  GLuint vertexBuffer;
  glGenBuffers(1, &vertexBuffer);

  printf("%u\n", vertexBuffer);

  while ( not glfwWindowShouldClose( window ) ) {
    glfwSwapBuffers( window );
    glfwPollEvents();
  }
}
