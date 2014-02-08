#ifndef GL_OBJECTS_HH
#define GL_OBJECTS_HH

#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <memory>

class GLFWContext
{
public:
  GLFWContext() { glfwInit(); }
  ~GLFWContext() { glfwTerminate(); }
};

class Window
{
  GLFWwindow * window_;

public:
  Window( const unsigned int width, const unsigned int height, const std::string & title );
  void make_context_current( void );
  bool should_close( void ) const;
  void swap_buffers( void );

  ~Window();

  /* forbid copy */
  Window( const Window & other ) = delete;
  Window & operator=( const Window & other ) = delete;
};

class VertexBuffer
{
  GLuint num_;

public:
  VertexBuffer() : num_() { glGenBuffers( 1, &num_ ); }
};

#endif /* GL_OBJECTS_HH */
