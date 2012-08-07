/* 
  Simple little program to print out the WKT for an .img file.
  This is used in the gdal common dll to produce pre defined
  files coordinate systems
  
  Sam Gillingham June 2004.
  
*/

#include <stdio.h>

#include "gdal.h"

int main( int argc, char *argv[] )
{
GDALDatasetH hHandle;
const char *pszWKT;
  
  if( argc != 2 )
  {
    fprintf( stderr, "usage: gdaldumpWKT filename\n" );
    exit( 1 ); 
  }
  
  GDALAllRegister();
  
  hHandle = GDALOpen( argv[1], GA_ReadOnly );
  
  pszWKT = GDALGetProjectionRef( hHandle );
  
  printf( "%s\n", pszWKT );
  
  GDALClose( hHandle );

  return 0;
}

