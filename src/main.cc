#include <stdexcept>
#include <random>
#include <iostream>

#include "graph.hh"

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

void glfun( int argc, char *argv[] )
{
  if ( argc < 1 ) {
    throw runtime_error( "missing argv[ 0 ]" );
  } else if ( argc != 1 ) {
    cerr << "Usage: " << argv[ 0 ] << endl;
    throw runtime_error( "bad command-line arguments" );
  }

  Graph graph( 1024, 768, "Ratatouille" );

  float t = 0;

  while ( true ) {
    t += 1.0 / 240.0;

    graph.set_window( t, 5.0 );

    graph.add_data_point( t, sin( t ) );

    if ( graph.blocking_draw( t, 5.0 ) ) {
      break;
    }
  }
}
