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
                            1.0 - 2 * position.y / window_size.y, 0.0, 1.0 );
        raw_position = vec2( position.x, window_size.y - position.y );
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

const std::string Display::shader_source_solid_color
= R"( #version 140

      uniform vec4 color;

      in vec2 raw_position;
      out vec4 outColor;

      void main()
      {
        outColor = color;
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

  /* set up shader programs. both use the same vertex shader */

  /* the texture shader program blits a texture to the screen */
  texture_shader_program_.attach( scale_from_pixel_coordinates_ );
  texture_shader_program_.attach( passthrough_texture_ );
  texture_shader_program_.link();
  glCheck( "after linking texture shader program" );

  /* the solid-color shader program just paints triangles of a given color */
  solid_color_shader_program_.attach( scale_from_pixel_coordinates_ );
  solid_color_shader_program_.attach( solid_color_ );
  solid_color_shader_program_.link();
  glCheck( "after linking solid-color shader program" );

  /* set up vertex array for corners of display */
  texture_shader_array_object_.bind();
  ArrayBuffer::bind( screen_corners_ );
  glVertexAttribPointer( texture_shader_program_.attribute_location( "position" ),
			 2, GL_FLOAT, GL_FALSE, 0, 0 );
  glEnableVertexAttribArray( texture_shader_program_.attribute_location( "position" ) );

  solid_color_array_object_.bind();
  ArrayBuffer::bind( other_vertices_ );
  glVertexAttribPointer( solid_color_shader_program_.attribute_location( "position" ),
			 2, GL_FLOAT, GL_FALSE, 0, 0 );
  glEnableVertexAttribArray( solid_color_shader_program_.attribute_location( "position" ) );
  glCheck( "after setting up vertex attribute arrays" );

  /* set sync-to-vblank */
  glfwSwapInterval( 1 );

  /* set up texture */
  texture_.bind();
  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  /* set size of viewport and tell shader program */
  const pair<unsigned int, unsigned int> window_size = window().size();
  resize( window_size );

  /* set up alpha-blending */
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  glCheck( "at end of Display constructor" );
}

void Display::resize( const pair<unsigned int, unsigned int> & target_size )
{
  /* set size of viewport and tell shader program */
  glViewport( 0, 0, target_size.first, target_size.second );

  texture_shader_program_.use();
  glUniform2ui( texture_shader_program_.uniform_location( "window_size" ),
		target_size.first, target_size.second );

  solid_color_shader_program_.use();
  glUniform2ui( solid_color_shader_program_.uniform_location( "window_size" ),
		target_size.first, target_size.second );

  /* load new coordinates of corners of image rectangle */
  const vector<pair<float, float>> corners = { { 0, 0 },
					       { 0, target_size.second },
					       { target_size.first, target_size.second },
					       { target_size.first, 0 } };
  texture_shader_array_object_.bind();
  ArrayBuffer::bind( screen_corners_ );
  ArrayBuffer::load( corners, GL_STATIC_DRAW );

  /* resize texture */
  texture_.resize( target_size.first, target_size.second );

  glCheck( "after resizing" );
}

void Display::draw( const Image & image )
{
  texture_.load( image );
  ArrayBuffer::bind( screen_corners_ );
  texture_shader_array_object_.bind();
  texture_shader_program_.use();
  glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
}

void Display::swap( void )
{
  current_context_window_.window_.swap_buffers();
}

void Display::draw( const float red, const float green, const float blue, const float alpha,
		    const float width,
		    const vector<pair<uint16_t, uint16_t>> & vertices )
{
  ArrayBuffer::bind( other_vertices_ );
  solid_color_array_object_.bind();
  solid_color_shader_program_.use();

  const float halfwidth = width / 2;

  vector<pair<float, float>> triangles;

  for ( auto it = vertices.begin(); it < vertices.end() - 1; it++ ) {
    const auto & start = *it;
    const auto & end = *(it + 1);

    /* horizontal portion */
    triangles.emplace_back( start.first - halfwidth, start.second - halfwidth );
    triangles.emplace_back( start.first - halfwidth, start.second + halfwidth );
    triangles.emplace_back( end.first - halfwidth, start.second + halfwidth );

    triangles.emplace_back( start.first - halfwidth, start.second - halfwidth );
    triangles.emplace_back( end.first - halfwidth, start.second - halfwidth );
    triangles.emplace_back( end.first - halfwidth, start.second + halfwidth );

    /* vertical portion */
    triangles.emplace_back( end.first - halfwidth, start.second - halfwidth );
    triangles.emplace_back( end.first - halfwidth, end.second - halfwidth );
    triangles.emplace_back( end.first + halfwidth, end.second - halfwidth );

    triangles.emplace_back( end.first - halfwidth, start.second - halfwidth );
    triangles.emplace_back( end.first + halfwidth, start.second - halfwidth );
    triangles.emplace_back( end.first + halfwidth, end.second - halfwidth );
  }

  if ( not vertices.empty() ) {
    /* fill in last square */
    triangles.emplace_back( vertices.back().first - halfwidth, vertices.back().second - halfwidth );
    triangles.emplace_back( vertices.back().first - halfwidth, vertices.back().second + halfwidth );
    triangles.emplace_back( vertices.back().first + halfwidth, vertices.back().second + halfwidth );

    triangles.emplace_back( vertices.back().first - halfwidth, vertices.back().second - halfwidth );
    triangles.emplace_back( vertices.back().first + halfwidth, vertices.back().second - halfwidth );   
    triangles.emplace_back( vertices.back().first + halfwidth, vertices.back().second + halfwidth );
  }

  solid_color_array_object_.bind();
  ArrayBuffer::bind( other_vertices_ );
  ArrayBuffer::load( triangles, GL_STREAM_DRAW );

  solid_color_shader_program_.use();
  glUniform4f( solid_color_shader_program_.uniform_location( "color" ),
	       red, green, blue, alpha );

  glDrawArrays( GL_TRIANGLES, 0, triangles.size() );
}

void Display::clear( void )
{
  glClear( GL_COLOR_BUFFER_BIT );
}
