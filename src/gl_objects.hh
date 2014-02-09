#ifndef GL_OBJECTS_HH
#define GL_OBJECTS_HH

#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

class GLFWContext
{
public:
  GLFWContext();
  ~GLFWContext();
};

class Window
{
  GLFWwindow * window_;

public:
  Window( const unsigned int width, const unsigned int height, const std::string & title );
  void make_context_current( const bool initialize_extensions = false );
  bool should_close( void ) const;
  void swap_buffers( void );
  bool key_pressed( const int key ) const;

  ~Window();

  /* forbid copy */
  Window( const Window & other ) = delete;
  Window & operator=( const Window & other ) = delete;
};

class VertexBufferObject
{
  GLuint num_;

public:
  VertexBufferObject();

  template <class BufferType>
  void bind( void ) { glBindBuffer( BufferType::id, num_ ); }

  /* forbid copy */
  VertexBufferObject( const VertexBufferObject & other ) = delete;
  VertexBufferObject & operator=( const VertexBufferObject & other ) = delete;
};

template <GLenum id_>
class Buffer
{
public:
  Buffer() = delete;
  static void load( const std::vector<std::pair<float, float>> & vertices, const GLenum usage )
  {
    glBufferData( id, vertices.size(), &vertices.front(), usage );
  }

  constexpr static GLenum id = id_;
};

using ArrayBuffer = Buffer<GL_ARRAY_BUFFER>;

void compile_shader( const GLuint num, const std::string & source );

template <GLenum type_>
class Shader
{
  GLuint num_ = glCreateShader( type_ );

public:
  Shader( const std::string & source )
  {
    compile_shader( num_, source );
  }

  /* forbid copy */
  Shader( const Shader & other ) = delete;
  Shader & operator=( const Shader & other ) = delete;
};

using VertexShader = Shader<GL_VERTEX_SHADER>;

#endif /* GL_OBJECTS_HH */
