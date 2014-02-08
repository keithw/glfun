#include <cstdlib>
#include <iostream>

using namespace std;

int main( int argc, char *argv[] )
{
  if ( argc != 1 ) {
    cerr << "Usage: " << argv[ 0 ] << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
