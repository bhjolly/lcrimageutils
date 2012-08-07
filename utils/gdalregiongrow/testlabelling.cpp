#define _XOPEN_SOURCE 500
#define _XOPEN_SOURECE 500

#include <stdio.h>
#include <stdlib.h>

#include "gdal_priv.h"
#include "gdal_frmts.h"

#include "gdalcommon_cpp.h"

#include "regionlabelling.h"

int main(int argc, char *argv[] )
{
GDALDataset *pInDataset, *pOutDataset;
GDALRasterBand *pInBand, *pOutBand;

  if( argc != 3 )
  {
    fprintf( stderr, "usage: testlabelling inimage outimage\n ");
    exit(1);
  }
  
  char *pszInImage = argv[1];
  char *pszOutImage = argv[2];

  GDALRegister_HFA();
  
  // Open the Grow Dataset
  pInDataset = (GDALDataset *) GDALOpen( pszInImage, GA_ReadOnly );
  if( pInDataset == NULL )
  {
    fprintf( stderr, "Cannot open %s\n", pszInImage );
    exit(1);
  }
  
  pInBand = pInDataset->GetRasterBand(1);

  // Create CMemRaster object and read data into it.
  int xsize = pInBand->GetXSize();
  int ysize = pInBand->GetYSize();

  CMemRaster<GByte> inRaster(xsize,ysize);
  pInBand->RasterIO( GF_Read, 0, 0, xsize, ysize, inRaster.getRaster(), 
                        xsize, ysize, GDT_Byte, 0, 0 );
  
  int last = CRegionLabelling::label(inRaster,true);
  printf( "Last value = %d\n", last );

   // Write out the output raster                        
  pOutDataset = (GDALDataset *)gdalcommon_newfile_templ((GDALDatasetH)pInDataset,pszOutImage,0,0,0,GDT_Unknown);
  if( pOutDataset == NULL )
  {
    fprintf( stderr, "Cannot open %s\n", pszOutImage );
    exit(1);
  }

  delete pInDataset;
  pInDataset = NULL;
  
  pOutBand = pOutDataset->GetRasterBand(1);
  pOutBand->RasterIO(GF_Write,0,0,xsize,ysize,inRaster.getRaster(),xsize,ysize,GDT_Byte,0,0);

  delete pOutDataset;
  pOutDataset = NULL;

  gdalcommon_calcstats( pszOutImage );

  return 0; 
}

