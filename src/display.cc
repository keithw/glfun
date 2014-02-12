#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "display.hh"

using namespace std;

const std::string Display::shader_source_scale_from_pixel_coordinates
= R"( #version 140

      uniform uvec2 window_size;
      in vec2 position;
      out vec2 raw_position;

      void main()
      {
	gl_Position = vec4( 2 * position.x / window_size.x - 1.0,
                            2 * position.y / window_size.y - 1.0, 0.0, 1.0 );
        raw_position = position;
      }
    )";

const std::string Display::shader_source_passthrough_texture
= R"( #version 140

      uniform sampler2DRect tex;

      in vec2 raw_position;
      out vec4 outColor;

      void main()
      {
        outColor = texture( tex, raw_position );
      }
    )";

Display::CurrentContextWindow::CurrentContextWindow( const unsigned int width, const unsigned int height,
						     const string & title )
  : window_( width, height, title )
{
  window_.make_context_current( true );
}

Display::Display( const unsigned int width, const unsigned int height,
		  const string & title )
  : current_context_window_( width, height, title ),
    texture_( width, height )
{
  glCheck( "starting Display constructor" );

  /* set up shader program to render texture in screen pixel coordinates */
  shader_program_.attach( scale_from_pixel_coordinates_ );
  shader_program_.attach( passthrough_texture_ );
  shader_program_.link();
  shader_program_.use();
  glCheck( "after installing shader program" );

  /* set up vertex array for corners of display */
  vertex_array_object_.bind();
  ArrayBuffer::bind( vertices_ );
  glVertexAttribPointer( shader_program_.attribute_location( "position" ),
			 2, GL_FLOAT, GL_FALSE, 0, 0 );
  glEnableVertexAttribArray( shader_program_.attribute_location( "position" ) );
  glCheck( "after setting up vertex attribute array" );

  /* set size of viewport and tell shader program */
  resize();

  /* set sync-to-vblank */
  glfwSwapInterval( 1 );

  /* set up texture */
  texture_.bind();
  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  glCheck( "end of Display constructor" );
}

void Display::resize( void )
{
  /* set size of viewport and tell shader program */
  const auto window_size = window().size();
  glViewport( 0, 0, window_size.first, window_size.second );
  glUniform2ui( shader_program_.uniform_location( "window_size" ),
		window_size.first, window_size.second );

  /* load new coordinates of corners of image rectangle */
  const vector<pair<float, float>> corners = { { 0, 0 },
					       { 0, window_size.second },
					       { window_size.first, window_size.second },
					       { window_size.first, 0 } };
  ArrayBuffer::load( corners, GL_STATIC_DRAW );

  /* resize texture */
  texture_.resize( window_size.first, window_size.second );
}

void Display::draw( const Image & image )
{
  texture_.load( image );
  repaint();
}

void Display::repaint( void )
{
  glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
  current_context_window_.window_.swap_buffers();
}
