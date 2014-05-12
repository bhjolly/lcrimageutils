
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gdal.h"

void gdalallregister_()
{
  GDALAllRegister();
}

/* =====================================================================*/
/* Major Object */
/* =====================================================================*/

void gdalsetdescription_(GDALMajorObjectH *obj, const char *desc, int desclen)
{
char *psz;

  psz = (char*)calloc( desclen + 1, sizeof(char) );
  
  strncpy( psz, desc, desclen );
  
  GDALSetDescription( *obj, psz );

  free( psz );
}

void gdalgetdescription_(GDALMajorObjectH *obj,char *desc, int desclen)
{
const char *psz;

  psz = GDALGetDescription(*obj);
  
  if( psz != NULL )
  {
    strncpy( desc, psz, desclen );
  }
  else
  {
    memset( desc, ' ', desclen );
  }
}


/* =====================================================================*/
/* Datasets */
/* =====================================================================*/

GDALDatasetH gdalopen_(const char *fname,int access,int fnamelen)
{
char *psz;
GDALDatasetH ds;

  psz = (char*)calloc( fnamelen + 1, sizeof(char) );
  
  strncpy( psz, fname, fnamelen );
  
  ds = GDALOpen( psz, (GDALAccess)access );

  free( psz );
  
  return ds;
}

GDALDatasetH gdalcreate_(GDALDatasetH *templ, const char *fname,int *xsize, int *ysize, 
                          int *bands, int *type,int fnamelen)
{
char *psz;
GDALDatasetH hds;
int n_xsize, n_ysize, n_bands, n_type;
double adfGeoTransform[6];
const char *pszWKT;
GDALDriverH hDriver;
char **pszStringList = NULL;

  psz = (char*)calloc( fnamelen + 1, sizeof(char) );
  
  strncpy( psz, fname, fnamelen );

  n_xsize = *xsize;
  n_ysize = *ysize;
  n_bands = *bands;
  n_type = *type;

  if( n_xsize == 0 )
  {
    n_xsize = GDALGetRasterXSize( *templ );
  }
  if( n_ysize == 0 )
  {
    n_ysize = GDALGetRasterYSize( *templ );
  }
  if( n_bands == 0 )
  {
    n_bands = GDALGetRasterCount( *templ );
  }
  if( n_type == GDT_Unknown )
  {
    n_type = GDALGetRasterDataType( GDALGetRasterBand( *templ, 1 ) );
  }

  if( GDALGetGeoTransform( templ, adfGeoTransform ) != CE_None )
  {
    fprintf( stderr, "Cannot find transform\n" );
    return NULL;
  }

  pszWKT = GDALGetProjectionRef( templ );

  /* Create the new file */
  hDriver = GDALGetDriverByName( "HFA" );
  if( hDriver == NULL )
  {
    fprintf( stderr, "Cannot find driver for HFA\n" );
    return NULL;
  }

  pszStringList = CSLAddString(NULL, "IGNOREUTM=TRUE");
  pszStringList = CSLAddString(pszStringList, "COMPRESSED=TRUE");
  hds = GDALCreate( hDriver, psz, n_xsize, n_ysize, n_bands, n_type, pszStringList );
  CSLDestroy( pszStringList );

  if( hds != NULL )
  {
    /* Set up the projection info */
    GDALSetProjection( hds, pszWKT );
    
    GDALSetGeoTransform( hds, adfGeoTransform );
  }

  free( psz );
  
  return hds;
}

void gdalclose_(GDALDatasetH *ds)
{
  GDALClose( *ds );
}

int gdalgetrasterxsize_(GDALDatasetH *ds)
{
  return GDALGetRasterXSize( *ds );
}

int gdalgetrasterysize_(GDALDatasetH *ds)
{
  return GDALGetRasterYSize( *ds );
}

int gdalgetrastercount_(GDALDatasetH *ds)
{
  return GDALGetRasterCount( *ds );
}

void gdalgetgeotransform_(GDALDatasetH *ds, double *transform)
{
  GDALGetGeoTransform( *ds, transform);
}

void gdalsetgeotransform_(GDALDatasetH *ds, double *transform)
{
  GDALSetGeoTransform( *ds, transform);
}

void gdalwld2pix_(double *transform,double *dwldx, double *dwldy, int *x, int *y)
{
double invtransform[6];
double dx, dy;

  GDALInvGeoTransform(transform, invtransform);
  GDALApplyGeoTransform(invtransform, *dwldx, *dwldy, &dx, &dy);
  *x = dx;
  *y = dy;
}

void gdalpix2wld_(double *transform, int *x, int *y,double *dwldx, double *dwldy)
{
  GDALApplyGeoTransform(transform, *x, *y, dwldx, dwldy);
}

void gdalsetprojection_(GDALDatasetH *ds, const char *proj, int projlen)
{
char *psz;

  psz = (char*)calloc( projlen + 1, sizeof(char) );
  
  strncpy( psz, proj, projlen );

  GDALSetProjection( *ds, psz );

  free( psz );
}

void gdalgetprojection_(GDALDatasetH *ds,char *proj, int projlen)
{
const char *psz;

  psz = GDALGetProjectionRef(*ds);

  if( psz != NULL )
  {
    strncpy( proj, psz, projlen );
  }
  else
  {
    memset( proj, ' ', projlen );
  }
}

GDALRasterBandH gdalgetrasterband_(GDALDatasetH *ds, int *band)
{
  return GDALGetRasterBand(*ds,*band);
}

/* =====================================================================*/
/* Bands */
/* =====================================================================*/

int gdalgetrasterdatatype_(GDALRasterBandH *bnd)
{
  return GDALGetRasterDataType(*bnd);
}

void gdalgetblocksize_(GDALRasterBandH *bnd, int *xsize, int *ysize)
{
  GDALGetBlockSize(*bnd,xsize,ysize);
}

void gdalrasterio_(GDALRasterBandH *bnd, int *flag, int *xoff, int *yoff,
                        int *xsize, int *ysize, void *buffer, int *bxsize, int *bysize,
                        int *type, int *pixelspace, int *linespace)
{
  GDALRasterIO( *bnd, (GDALRWFlag)*flag, *xoff, *yoff, *xsize, *ysize, buffer, *bxsize, *bysize,
                (GDALDataType)*type, *pixelspace, *linespace);
}

void gdalreadblock_(GDALRasterBandH *bnd, int *x, int *y, void *buffer)
{
  GDALReadBlock(*bnd,*x,*y,buffer);
}

void gdalwriteblock_(GDALRasterBandH *bnd, int *x, int *y, void *buffer)
{
  GDALWriteBlock(*bnd,*x,*y,buffer);
}

int gdalgetrasterbandxsize_(GDALRasterBandH *bnd)
{
  return GDALGetRasterBandXSize(*bnd);
}

int gdalgetrasterbandysize_(GDALRasterBandH *bnd)
{
  return GDALGetRasterBandYSize(*bnd);
}

void gdalflushrastercache_(GDALRasterBandH *bnd)
{
  GDALFlushRasterCache(*bnd);
}

void gdalcomputerasterminmax_(GDALRasterBandH *bnd, int *approxok, double *minmax)
{
  GDALComputeRasterMinMax(*bnd,*approxok,minmax);
}

void gdalgetrasterhistogram_(GDALRasterBandH *bnd, double *min, double *max, int *buckets,
                              int *histo, int *includeoutofrange, int *approxok)
{
  GDALGetRasterHistogram( *bnd, *min, *max, *buckets, histo, *includeoutofrange,
                          *approxok, GDALDummyProgress, NULL );
}

void gdalcomputebandstats_(GDALRasterBandH *bnd,int *step,double *mean,double *stddev)
{
  GDALComputeBandStats(*bnd,*step,mean,stddev,GDALDummyProgress, NULL );
}
