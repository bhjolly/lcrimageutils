
/* Fancy program to degrade an image by a certain factor.
 ie factor 4 on a 25m pixel image will make them 100m
 
 Sam Gillingham October 2005
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* GDAL stuff */
#include "gdal.h"
#include "gdal_frmts.h"
#include "cpl_conv.h"

#include "gdalcommon.h"

int main( int argc, char *argv[] )
{
GDALDatasetH hDstDS, hSrcDS;
GDALRasterBandH hDstBand, hSrcBand;
int inxsize, inysize, outxsize, outysize;
int percent, lastpercent;
int band;
GByte *pbyteBuffer = NULL;
GUInt16 *puint16Buffer = NULL;
double adfGeoTransform[6];
int y;
int factor;
GDALDataType type;

  if( argc != 4 )
  {
    fprintf( stderr, "usage: gdaldegrade infile outfile factor\n" );
    exit(1);
  }

  factor = atoi(argv[3]);

  /* Initialize connection to GDAL */
  //GDALRegister_HFA();
  GDALAllRegister();

  /* Open the input dataset */
  hSrcDS = GDALOpen( argv[1], GA_ReadOnly );

  if( hSrcDS == NULL ) {
    fprintf(stderr,"ERROR: couldn't open %s\n",argv[1]);
  }

  inxsize = GDALGetRasterXSize( hSrcDS );
  inysize = GDALGetRasterYSize( hSrcDS );
  
  outxsize = inxsize / factor;
  outysize = inysize / factor;
  
  fprintf( stderr, "insize = %d,%d outsize = %d,%d\n", inxsize, inysize, outxsize, outysize );

  type = GDALGetRasterDataType( GDALGetRasterBand(hSrcDS, 1 ) );
  if( ( type != GDT_Byte ) && ( type != GDT_UInt16 ) )
  {
    fprintf( stderr, "Datatype not supported\n" );
    exit( 1 );
  }

  /* Create new img file based on the old one */
  hDstDS = gdalcommon_new( argv[2], outxsize, outysize, GDALGetRasterCount(hSrcDS), type );
  if( hDstDS == NULL )
  {
    fprintf( stderr, "Cannot create %s\n",  argv[2] );
    exit( 1 );
  }
 
  if( GDALGetGeoTransform( hSrcDS, adfGeoTransform ) != CE_None )
  {
    fprintf( stderr, "Cannot find coordinate info %s\n", argv[1] );
    exit( 1 );
  }
  
  adfGeoTransform[1] = adfGeoTransform[1] * factor;
  adfGeoTransform[5] = adfGeoTransform[5] * factor;

  GDALSetProjection( hDstDS, GDALGetProjectionRef( hSrcDS ) );
  
  GDALSetGeoTransform( hDstDS, adfGeoTransform );

  if( type == GDT_Byte )
  {
    pbyteBuffer = (GByte*)calloc( sizeof(GByte),outxsize ); 
  }
  else
  {
    puint16Buffer = (GUInt16*)calloc( sizeof(GUInt16),outxsize ); 
  }

  /* Go through each band */
  for( band = 0; band < GDALGetRasterCount( hSrcDS ); band++ )
  {
    fprintf( stderr, "Band %d\n", band + 1 );
  
    hSrcBand = GDALGetRasterBand( hSrcDS, band + 1 );
    hDstBand = GDALGetRasterBand( hDstDS, band + 1 );
    
    lastpercent = -1;
    for( y = 0; y < outysize; y++ )
    {
      percent = (int)( (double)y / (double)outysize * 100.0 );
      if( percent != lastpercent )
      {
        fprintf( stderr, "%d%%\r", percent );
        lastpercent = percent;
     }
    
      if( type == GDT_Byte )
      {
        GDALRasterIO( hSrcBand, GF_Read, 0, y*factor, inxsize, 1, pbyteBuffer, outxsize, 1, GDT_Byte,1, inxsize );
    
        GDALRasterIO( hDstBand, GF_Write, 0, y, outxsize, 1, pbyteBuffer, outxsize, 1, GDT_Byte, 1, outxsize );
      }
      else
      {
        GDALRasterIO( hSrcBand, GF_Read, 0, y*factor, inxsize, 1, puint16Buffer, outxsize , 1, GDT_UInt16, sizeof(GUInt16), inxsize  * sizeof(GUInt16));
    
        GDALRasterIO( hDstBand, GF_Write, 0, y, outxsize, 1, puint16Buffer, outxsize, 1, GDT_UInt16, sizeof(GUInt16), outxsize  * sizeof(GUInt16));
      }
    }
  }
  
  if( type == GDT_Byte )
  {
    free( pbyteBuffer );
  }
  else
  {
    free( puint16Buffer );
  }
  
  GDALClose( hSrcDS );
  GDALClose( hDstDS );

  fprintf( stderr, "Calculating Statistics:\n" );

  gdalcommon_calcstatsignore( argv[2], 0 );
  
  return 0;
}
