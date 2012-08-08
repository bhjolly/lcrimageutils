
#include <stdlib.h>
#include <math.h>

#include "gdal.h"
#include "gdalcommon.h"

void printusage()
{
  fprintf( stderr, "usage: gdalcalcstats filename [-ignore val]\n" );
  exit( 1 ); 
}


int main(int argc, char *argv[] )
{
int i;
bool ignore = false;
float ignoreval = 0;
    
  if( ( argc < 2 ) || ( argc > 4 ) )
  {
    printusage();
  }
  
  /* Check the command line */
  for( i = 2; i < argc; i++ )
  {
    if( strcmp( argv[i], "-ignore" ) == 0 )
    {
      ignore = true;
      i++;
      if( i == argc )
      {
        /* they haven't given us the value */
        printusage(); 
      }
      else
      {
        ignoreval = atof( argv[i] );        
      }
    }
		else
		{
			printusage();
		}
  }
  
  GDALAllRegister();

  // Open the file 
  GDALDatasetH inds = GDALOpen( argv[1], GA_Update );
  if( inds != NULL )
  {
    // calc the stats 
    if( ignore )
        gdalcommon_calcstatsignore( inds, ignoreval );
    else
        gdalcommon_calcstats( inds );
  
    GDALClose(inds);
    return 0;
  }
  else
  {
    fprintf( stderr, "Cannot open %s\n", argv[1] );
    return 1;
  }
  
}

