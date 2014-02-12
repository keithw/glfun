#ifndef DISPLAY_HH
#define DISPLAY_HH

#include <vector>
#include <string>

#include "gl_objects.hh"

class Display
{
  static const std::string shader_source_scale_from_pixel_coordinates;
  static const std::string shader_source_passthrough_texture;

  struct CurrentContextWindow
  {
    GLFWContext glfw_context_ = {};
    Window window_;

    CurrentContextWindow( const unsigned int width, const unsigned int height,
			  const std::string & title );
  } current_context_window_;

  VertexShader scale_from_pixel_coordinates_ = { shader_source_scale_from_pixel_coordinates };
  FragmentShader passthrough_texture_ = { shader_source_passthrough_texture };
  Program shader_program_ = {};

  Texture texture_;

  VertexArrayObject vertex_array_object_ = {};
  VertexBufferObject vertices_ = {};

  void resize( void );

public:
  Display( const unsigned int width, const unsigned int height,
	   const std::string & title );

  void draw( const Image & image );

  void repaint( void );

  const Window & window( void ) const { return current_context_window_.window_; }
};

#endif /* DISPLAY_HH */
