#define _XOPEN_SOURCE 500
#define _XOPEN_SOURECE 500

#include <stdio.h>
#include <stdlib.h>

#include "gdal_priv.h"
#include "gdal_frmts.h"

#include "gdalcommon_cpp.h"

/*===================================================================================*/


int main(int argc,char *argv[] )
{
GDALDataset *pGrowDataset, *pMaskDataset, *pOutDataset;
GDALRasterBand *pGrowBand, *pMaskBand, *pOutBand;
  
  if( argc != 7 )
  {
    fprintf( stderr, "usage: gdalregiongrow growimage band maskimage min max outimage\n ");
    exit(1);
  }
  
  char *pszGrowImage = argv[1];
  int nGrowBand = atoi(argv[2]);
  char *pszMaskImage = argv[3];
  int nMin = atoi(argv[4]);
  int nMax = atoi(argv[5]);
  char *pszOutImage = argv[6];
  
  GDALRegister_HFA();
  
  // Open the Grow Dataset
  pGrowDataset = (GDALDataset *) GDALOpen( pszGrowImage, GA_ReadOnly );
  if( pGrowDataset == NULL )
  {
    fprintf( stderr, "Cannot open %s\n", pszGrowImage );
    exit(1);
  }
  
  pGrowBand = pGrowDataset->GetRasterBand(nGrowBand);
  
  // Open the Mask Dataset
  pMaskDataset = (GDALDataset *) GDALOpen( pszMaskImage, GA_ReadOnly );
  if( pMaskDataset == NULL )
  {
    fprintf( stderr, "Cannot open %s\n", pszMaskImage );
    exit(1);
  }
  
  pMaskBand = pMaskDataset->GetRasterBand(1);
  
  // Check the sizes are the same.
  if( (pGrowBand->GetXSize() != pMaskBand->GetXSize() ) || ( pGrowBand->GetYSize() != pMaskBand->GetYSize() ) )
  {
    fprintf( stderr, "Raster sizes do not match\n" );
    exit(1); 
  }
  
  // Create CMemRaster object and read data into it.
  int xsize = pGrowBand->GetXSize();
  int ysize = pGrowBand->GetYSize();
  
  CMemRaster<GByte> growRaster(xsize,ysize);
  pGrowBand->RasterIO( GF_Read, 0, 0, xsize, ysize, growRaster.getRaster(), 
                        xsize, ysize, GDT_Byte, 0, 0 );
  delete pGrowDataset;
  pGrowDataset = NULL;
                        
  // Create the output raster object
  CMemRaster<GByte> outRaster(xsize,ysize);        
  
  // Create object that does the work
  CRegionGrow<GByte> regionGrow(&growRaster,&outRaster,nMin,nMax);
  
  // Create a line for the mask
  CMemRaster<GByte> maskRaster(xsize,1);
  for( int row = 0; row < ysize; row++ )
  {
    // Read the mask a line at a time
    pMaskBand->RasterIO( GF_Read, 0, row, xsize, 1, maskRaster.getRaster(),
                          xsize, 1, GDT_Byte,1, xsize );
                          
    for( int col = 0; col < xsize; col++ )
    {
      if( maskRaster.getValue(col,0) != 0 )
      {
        regionGrow.growRegion(col,row);
      }
    }   
  }

  // Write out the output raster                        
  pOutDataset = (GDALDataset *)gdalcommon_newfile_templ((GDALDatasetH)pMaskDataset,pszOutImage,0,0,0,GDT_Unknown);
  if( pOutDataset == NULL )
  {
    fprintf( stderr, "Cannot open %s\n", pszOutImage );
    exit(1);
  }
  
  delete pMaskDataset;
  pMaskDataset = NULL;

  pOutBand = pOutDataset->GetRasterBand(1);
  pOutBand->RasterIO(GF_Write,0,0,xsize,ysize,outRaster.getRaster(),xsize,ysize,GDT_Byte,0,0);

  delete pOutDataset;
  pOutDataset = NULL;

	fprintf( stderr, "Calculating Statistics...\n" );

  gdalcommon_calcstats( pszOutImage );
  return 0;
}
