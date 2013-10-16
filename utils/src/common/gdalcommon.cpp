
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <ctime>

#include "gdal.h"
#include "cpl_string.h"
#include "gdal_priv.h"

#include "gdalcommon_cpp.h"

void calcstats( GDALDataset *hHandle, bool bIgnore, float fIgnoreVal, bool bPyramid );

/* converts from coordinates to pixels */
void gdalcommon_wld2pix( double *adfGeoTransform, double dGeoX, double dGeoY, int *nX, int *nY )
{
  *nX = int(( adfGeoTransform[0] * adfGeoTransform[5] - 
        adfGeoTransform[2] * adfGeoTransform[3] + adfGeoTransform[2] * dGeoY - 
        adfGeoTransform[5] * dGeoX ) /
       ( adfGeoTransform[2] * adfGeoTransform[4] - adfGeoTransform[1] * adfGeoTransform[5] ));
       
  *nY = int(( adfGeoTransform[1] * adfGeoTransform[3] - adfGeoTransform[0] * adfGeoTransform[4] -
        adfGeoTransform[1] * dGeoY + adfGeoTransform[4] * dGeoX ) /
        ( adfGeoTransform[2] * adfGeoTransform[4] - adfGeoTransform[1] * adfGeoTransform[5] ));
}

/* converts from pixels to coordinates */
void gdalcommon_pix2wld( double *adfGeoTransform, int nX, int nY, double *dGeoX, double *dGeoY )
{
  *dGeoX = adfGeoTransform[0] + adfGeoTransform[1] * nX
          + adfGeoTransform[2] * nY;
  *dGeoY = adfGeoTransform[3] + adfGeoTransform[4] * nX
          + adfGeoTransform[5] * nY;
}

char** gdalcommon_create_options()
{
char **pszStringList = NULL;

  /* Create String with dodgy hack that Frank put in for us that allows 
  Files to be creates as Transverse Mercator */
  pszStringList = CSLAddString(NULL, "IGNOREUTM=TRUE");
  
  /* My Dodgy hack to ensure file always written out compressed*/
  pszStringList = CSLAddString(pszStringList, "COMPRESS=TRUE");
  
  return pszStringList;		
}

/* Creates a new Image with SLATS specific parameters */
GDALDatasetH gdalcommon_new(const char *szName, int xsize, int ysize, int bands, GDALDataType type)
{
GDALDriverH hDriver;
GDALDatasetH hDstDS;
char **pszStringList = NULL;

  hDriver = GDALGetDriverByName( "HFA" );
  if( hDriver == NULL )
  {
    fprintf( stderr, "Cannot find driver for HFA\n" );
    return NULL;
  }
  
  pszStringList = gdalcommon_create_options();
  
  /* Create the new file */
  hDstDS = GDALCreate( hDriver, szName, xsize, ysize, bands, type, pszStringList );
  
  CSLDestroy( pszStringList );
  
  return hDstDS;
}

/* Creates a new file based on existing. If param in zero then same as templ */
GDALDatasetH gdalcommon_newfile_templ( GDALDatasetH hTempl, const char *szName, int xsize, int ysize, int bands, GDALDataType type )
{
double adfGeoTransform[6];
const char *pszWKT;
GDALDatasetH hDstDS = NULL;

  if( xsize == 0 )
  {
    xsize = GDALGetRasterXSize( hTempl );
  }
  if( ysize == 0 )
  {
    ysize = GDALGetRasterYSize( hTempl );
  }
  if( bands == 0 )
  {
    bands = GDALGetRasterCount( hTempl );
  }
  if( type == GDT_Unknown )
  {
    type = GDALGetRasterDataType( GDALGetRasterBand( hTempl, 1 ) );
  }

  if( GDALGetGeoTransform( hTempl, adfGeoTransform ) != CE_None )
  {
    fprintf( stderr, "Cannot find transform\n" );
    return NULL;
  }

  pszWKT = GDALGetProjectionRef( hTempl );

  /* Create the new file */
  hDstDS = gdalcommon_new(szName, xsize, ysize, bands, type);

  if( hDstDS != NULL )
  {
    /* Set up the projection info */
    GDALSetProjection( hDstDS, pszWKT );
    
    GDALSetGeoTransform( hDstDS, adfGeoTransform );
  }

  return hDstDS;
}

/* Creates a brand new file not based on a current file */
/* Zone is the UTM zone - 0 for Lat/Long */
GDALDatasetH gdalcommon_newimg( const char *pszInName, int nXSize, int nYSize, int nBands, GDALDataType eBandType,
                                double dTLX, double dTLY, double dXpix, double dYPix, int nZone )
{
GDALDatasetH hHandle;
double adfGeoTransform[6];
const char *pszWKT = NULL;

  hHandle = gdalcommon_new( pszInName, nXSize, nYSize, nBands, eBandType );
  
  /* Set coord info */
  if( hHandle != NULL )
  {
    adfGeoTransform[0] = dTLX;
    adfGeoTransform[1] = dXpix;
    adfGeoTransform[2] = 0;
    adfGeoTransform[3] = dTLY;
    adfGeoTransform[4] = 0;
    adfGeoTransform[5] = dYPix;
    
    GDALSetGeoTransform( hHandle, adfGeoTransform );
    
    switch( nZone )
    {
     case 0:
      /* Lat/Long */
      pszWKT = "GEOGCS[\"WGS_1984\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.2572235629972],TOWGS84[0,0,0,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]]";
      break;
      
     case 54:
      /* UTM Zone 54 */
      pszWKT = "PROJCS[\"Transverse Mercator\",GEOGCS[\"GDA94\",DATUM[\"GDA94\",SPHEROID[\"GRS 1980\",6378137,298.2572220960422],TOWGS84[-16.237,3.51,9.939,1.4157e-06,2.1477e-06,1.3429e-06,1.91e-07]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],UNIT[\"meters\",1],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",141],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",10000000]]";
      break;
      
     case 55:
      /* UTM Zone 55 */
      pszWKT = "PROJCS[\"Transverse Mercator\",GEOGCS[\"GDA94\",DATUM[\"GDA94\",SPHEROID[\"GRS 1980\",6378137,298.2572220960422],TOWGS84[-16.237,3.51,9.939,1.4157e-06,2.1477e-06,1.3429e-06,1.91e-07]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],UNIT[\"meters\",1],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",147],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",10000000]]";
      break;
      
     case 56:
      /* UTM Zone 56 */
      pszWKT = "PROJCS[\"Transverse Mercator\",GEOGCS[\"GDA94\",DATUM[\"GDA94\",SPHEROID[\"GRS 1980\",6378137,298.2572220960422],TOWGS84[-16.237,3.51,9.939,1.4157e-06,2.1477e-06,1.3429e-06,1.91e-07]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],UNIT[\"meters\",1],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",153],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",10000000]]";
      break;
      
     default:
      fprintf( stderr, "Unknown zone %d\n", nZone );
      break;
    }
    
    GDALSetProjection( hHandle, pszWKT );
  }
  
  
  return hHandle;
}


void gdalcommon_calcstats( GDALDatasetH hDataset )
{
  calcstats( (GDALDataset*)hDataset, false, 0 , true );
}

void gdalcommon_calcstatsignore( GDALDatasetH hDataset, double dVal )
{
  calcstats( (GDALDataset*)hDataset, true, dVal, true );
}

int StatsTextProgress( double dfComplete, const char *pszMessage, void *pData)
{
    int nPercent = int(dfComplete*100);
    int *pnLastComplete = (int*)pData;
    if( nPercent != *pnLastComplete )
    {
        if( pszMessage != NULL )
            printf( "%d%% complete: %s\r", nPercent, pszMessage );
        else
            printf( "%d%% complete.\r", nPercent );

        *pnLastComplete = nPercent;
    }
    return TRUE;
}


/* we don't want to build unnecessarily small overview layers  */
/* we stop when the smallest dimension in the overview is less */
/* than this number                                            */
#define MINOVERVIEWDIM 33


void addpyramid( GDALDataset *handle )
{
/* These are the levels Imagine seems to use */

/* The levels used depend on the size of the image */


int nLevels[] = { 4, 8, 16, 32, 64, 128, 256, 512 };
int nOverviews,mindim,i;
GDALRasterBand *hBand;
const char *pszType;
int nLastProgress = -1;

/* first we work out how many overviews to build based on the size */
 mindim = handle->GetRasterXSize() < handle->GetRasterYSize() ? handle->GetRasterXSize() : handle->GetRasterYSize();

 nOverviews = 0;
 for(i=0;i< 8;i++) {
   if( (mindim/nLevels[i]) > MINOVERVIEWDIM ) ++nOverviews;
 }

	/* Need to find out if we are thematic or continuous */
	hBand = handle->GetRasterBand( 1 );
	
	if( strcmp( hBand->GetMetadataItem( "LAYER_TYPE", "" ), "athematic" ) == 0 )
	{
		pszType = "AVERAGE";
	}
	else
	{
		pszType = "NEAREST";
	}
	
  handle->BuildOverviews( pszType, nOverviews, nLevels, 0, NULL, StatsTextProgress, &nLastProgress );
    std::cout << std::endl;
}

void getRangeMean(float *pData,int size,float &min,float &max,float &mean, bool ignore, float ignoreVal)
{
  bool bFirst = true;
  double dTot = 0;
  int total_used = 0;
  float val;
  for( int count = 0; count < size; count++ )
  {
	  val = pData[count];
      if( !CPLIsNan(val ) )
      {
        if( !ignore || val != ignoreVal )
	    {
          dTot += val;
          total_used++;
		  if( bFirst )
		  {
		    max = val;
		    min = val;
		    bFirst = false;
		  }
		  else
		  {
		    if( val > max )
  		    {
    	      max = val;
      	    }
 	        else if( val < min )
 		    {
	          min = val;
	        }
          }
		}
	  }
   }
   mean = dTot / double(total_used);
}

float getStdDev(float *pData, int size, float fmean, bool ignore, float ignoreVal)
{
  double dVariance = 0;
  double dTot = 0;
  float val;
  int total_used = 0;
  for( int count = 0; count < size; count++ )
  {
    val = pData[count];
		if( !ignore || val != ignoreVal )
		{
      double diff = val - fmean;
      dTot += diff;
      dVariance += diff * diff;
      total_used++;
    }
  }
  if( total_used > 1 )
  {
    dVariance = (dVariance - dTot*dTot/total_used)/(total_used -1);
  	return sqrt(dVariance);
  }
  else
  {
    return 0;
  }
}
  
float* getSubSampledImage( GDALRasterBand *hBand, int nLevel, int *pnSize )
{
/*
  Our own "pyramid" layer code that does a nearest neighbour resample.
  Can't use the GDAL pyramid layers as they are generally AVERAGE
  and can't seem to trick GDAL into making them in a seperate 
  file for us using GDALRegenerateOverviews.
  Creates a single dimension array which must be free()ed by the caller.
*/
int inxsize, inysize, outxsize, outysize, y;
float *pData;

  inxsize = hBand->GetXSize();
  inysize = hBand->GetYSize();
  
  outxsize = inxsize / nLevel;
  outysize = inysize / nLevel;

  pData = (float*)malloc( outxsize * outysize * sizeof(float));
  *pnSize = outxsize * outysize;
  
  for( y = 0; y < outysize; y++ )
  {
    hBand->RasterIO( GF_Read, 0, y*nLevel, inxsize, 1, &pData[y*outxsize], outxsize, 1, GDT_Float32,0, 0 );
  }
  
  
  return pData;
}
  
#define HISTO_NBINS 256
/* the min. number of points to sample to get the stats */

#define MAX_TEMP_STRING 64

void calcstats( GDALDataset *hHandle, bool bIgnore, float fIgnoreVal, bool bPyramid )
{
    int band;//, xsize, ysize, osize,nlevel;
    GDALRasterBand *hBand;
    std::string histoType;
    std::string histoTypeDirect = "direct";
    std::string histoTypeLinear = "linear";
    char szTemp[MAX_TEMP_STRING];
    int nLastProgress;

  /* we calculate a single stats on full res - maybe we should use overviews for large datasets */
 
  int nBands = hHandle->GetRasterCount();
  for( band = 0; band < nBands; band++ )
  {
      std::cout << "Processing band " << band+1 << " of " << nBands << std::endl;
      
      hBand = hHandle->GetRasterBand( band + 1 );

      if( bIgnore )
      {
          hBand->SetNoDataValue( fIgnoreVal );
          snprintf( szTemp, MAX_TEMP_STRING, "%f", fIgnoreVal );
          GDALSetMetadataItem( hBand, "STATISTICS_EXCLUDEDVALUES", szTemp, NULL );
      }
    
      /* Find min, max, mean and stddev */ 
      double fmin=0, fmax=0, fMean=0, fStdDev=0;
      nLastProgress = -1;
      hBand->ComputeStatistics(false, &fmin, &fmax, &fMean, &fStdDev, StatsTextProgress, &nLastProgress);
      std::cout << std::endl;
      
	  /* Write Statistics */
  	snprintf( szTemp, MAX_TEMP_STRING, "%f", fmin );
  	hBand->SetMetadataItem( "STATISTICS_MINIMUM", szTemp, NULL );

    snprintf( szTemp, MAX_TEMP_STRING, "%f", fmax );
  	hBand->SetMetadataItem( "STATISTICS_MAXIMUM", szTemp, NULL );

 	snprintf( szTemp, MAX_TEMP_STRING, "%f", fMean );
  	hBand->SetMetadataItem( "STATISTICS_MEAN", szTemp, NULL );

 	snprintf( szTemp, MAX_TEMP_STRING, "%f", fStdDev );
  	hBand->SetMetadataItem( "STATISTICS_STDDEV", szTemp, NULL );

    /* as we calculated on full res - these are the default anyway */
    hBand->SetMetadataItem( "STATISTICS_SKIPFACTORX", "1", NULL );
    hBand->SetMetadataItem( "STATISTICS_SKIPFACTORY", "1", NULL );
   
    /* make sure that the histogram will work even if there */
    /* is only one value in it                              */
    fmin = fmin == fmax ? fmin - 1e-05 : fmin;

    /*fprintf( stderr, "doing histo\n" );*/
     
    /* Calc the histogram */
    int *pHisto;
    int nHistBuckets;
      
      float histmin = 0, histmax = 0, histminTmp = 0, histmaxTmp = 0;
      if( hBand->GetRasterDataType() == GDT_Byte )
      {
          /* if it is 8 bit just do a histo on the lot so we don't get rounding errors */ 
          nHistBuckets = 256;
          histmin = 0;
          histmax = 255;
          histminTmp = -0.5;
          histmaxTmp = 255.5;
          snprintf( szTemp, MAX_TEMP_STRING, "%f", fStdDev );
          histoType = histoTypeDirect;
      }
      else if(strcmp( hBand->GetMetadataItem( "LAYER_TYPE", "" ), "thematic" ) == 0)
      {
          nHistBuckets = ceil(fmax)+1;
          histmin = 0;
          histmax = ceil(fmax);
          histminTmp = -0.5;
          histmaxTmp = histmax + 0.5;
          histoType = histoTypeDirect;
      }
      else
      {
          int range = (ceil(fmax) - floor(fmin));
          histmin = fmin;
          histmax = fmax;
          if((range > 0) && (range <= HISTO_NBINS))
          {
              nHistBuckets = range;
              histoType = histoTypeDirect;
              
              histminTmp = histmin - 0.5;
              histmaxTmp = histmax + 0.5;
          }
          else
          {
              nHistBuckets = HISTO_NBINS;
              histoType = histoTypeLinear;
              histminTmp = histmin;
              histmaxTmp = histmax;
          }
      }
      pHisto = (int*)calloc(nHistBuckets, sizeof(int));
      // the patch 005_histoignore.patch means that ignore values are ignored
      // (if set) in calculation of histogram
      nLastProgress = -1;
      hBand->GetHistogram(histminTmp, histmaxTmp, nHistBuckets, pHisto, true, false, StatsTextProgress, &nLastProgress);
      std::cout << std::endl;
        
    /* Mode is the bin with the highest count */
    int maxcount = 0;
    int maxbin = 0;
    long totalvalues = 0;
    int nTotalSizeStringBinValues = 0;
    for( int count = 0; count < nHistBuckets; count++ )
    {
        if( pHisto[count] > maxcount )
        {
            maxcount = pHisto[count];
            maxbin = count;
        }
        totalvalues += pHisto[count];

        /* Work out the size needed for the histogram string */
        /* by adding together the size needed for each bin */
        nTotalSizeStringBinValues += snprintf(NULL, 0, "%d|", pHisto[count] );
    }
    float fMode = maxbin * ((histmax-histmin) / nHistBuckets);
    
    if( ( hBand->GetRasterDataType( ) == GDT_Float32 ) || 
      ( hBand->GetRasterDataType( ) == GDT_Float64 ) )
    {
  	    snprintf( szTemp, MAX_TEMP_STRING, "%f", fMode );
    }
    else
    {
  	    snprintf( szTemp, MAX_TEMP_STRING, "%f", floor(fMode+0.5) );
    }  
  	GDALSetMetadataItem( hBand, "STATISTICS_MODE", szTemp, NULL );

    /* Allocate space for histogram string */
    char *pszHistoString = new char[nTotalSizeStringBinValues + 1];
    int nHistoStringCurrentIdx = 0;

    int nWhichMedian = -1;
    int nCumSum = 0;
    for( int count = 0; count < nHistBuckets; count++ )
  	{
	    /* we also estimate the median based on the histogram */
	    if(nWhichMedian == -1) 
        {
	        /* then haven't found the median yet */
  	        nCumSum += pHisto[count];
	        if(nCumSum > (totalvalues/2.0)) 
            {
	            nWhichMedian = count;
            }
	    }
	    nHistoStringCurrentIdx += sprintf( &pszHistoString[nHistoStringCurrentIdx], "%d|", pHisto[count] );
  	}
   
    float fMedian = nWhichMedian * ((histmax-histmin) / nHistBuckets);
    if( nWhichMedian < nHistBuckets - 1 )
    {
      fMedian = ( fMedian + ((nWhichMedian+1) * ((histmax-histmin) / nHistBuckets)) ) / 2;
    }

    if( ( GDALGetRasterDataType( hBand ) == GDT_Float32 ) || 
      ( GDALGetRasterDataType( hBand ) == GDT_Float64 ) )
    {
    	snprintf( szTemp, MAX_TEMP_STRING, "%f", fMedian );
    }
    else
    {
    	snprintf( szTemp, MAX_TEMP_STRING, "%f", floor(fMedian+0.5) );
    }
	  GDALSetMetadataItem( hBand, "STATISTICS_MEDIAN", szTemp, NULL );

    if( GDALGetRasterDataType( hBand ) == GDT_Byte )
    {
  	  snprintf( szTemp, MAX_TEMP_STRING, "0" );
    }
    else
    {
  	  snprintf( szTemp, MAX_TEMP_STRING, "%f", histmin );
    }
  	GDALSetMetadataItem( hBand, "STATISTICS_HISTOMIN", szTemp, NULL );

    if( GDALGetRasterDataType( hBand ) == GDT_Byte )
    {
  	  snprintf( szTemp, MAX_TEMP_STRING, "255" );
    }
    else
    {
  	  snprintf( szTemp, MAX_TEMP_STRING, "%f", histmax );
    }
  	GDALSetMetadataItem( hBand, "STATISTICS_HISTOMAX", szTemp, NULL );

  	snprintf( szTemp, MAX_TEMP_STRING, "%d", nHistBuckets );
    GDALSetMetadataItem( hBand, "STATISTICS_HISTONUMBINS", szTemp, NULL );

  	GDALSetMetadataItem( hBand, "STATISTICS_HISTOBINVALUES", pszHistoString, NULL );
    
    /* set histogram bin function the same as Imagine does
     ie Linear for floats, and direct for integer types
     This means Raster Attribute Dialog gets displayed
     correctly in Imagine */
    GDALSetMetadataItem( hBand, "STATISTICS_HISTOBINFUNCTION", histoType.c_str(), NULL );
    
    if(strcmp( hBand->GetMetadataItem( "LAYER_TYPE", "" ), "thematic" ) == 0)
    {
        GDALColorTable *clrTab = hBand->GetColorTable();
        if(clrTab == NULL)
        {
            std::cout << "Adding a Colour table\n";
            clrTab = new GDALColorTable();
            GDALColorEntry clr;
            clr.c4 = 255;
            srand(time(NULL));
            for(int i = 0; i < nHistBuckets; ++i)
            {
                clr.c1 = rand() % 255 + 1;
                clr.c2 = rand() % 255 + 1;
                clr.c3 = rand() % 255 + 1;
                clrTab->SetColorEntry(i, &clr);
            }
            hBand->SetColorTable(clrTab);
            delete clrTab;
        }
    }
      
    delete pszHistoString;
    free( pHisto );
  }
  if( bPyramid )
  {
      std::cout << "Calculating Pyramids:\n";
    addpyramid(hHandle);
  }
}
 
