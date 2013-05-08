
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gdal.h"
#include "gdalcommon.h"

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

  psz = (char*)calloc( fnamelen + 1, sizeof(char) );
  
  strncpy( psz, fname, fnamelen );

  hds = gdalcommon_newfile_templ( *templ, psz, *xsize, *ysize, *bands, (GDALDataType)*type );

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
  gdalcommon_wld2pix(transform,*dwldx,*dwldy,x,y);
}

void gdalpix2wld_(double *transform, int *x, int *y,double *dwldx, double *dwldy)
{
  gdalcommon_pix2wld( transform, *x, *y, dwldx, dwldy );
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

void gdalcalcstats_(GDALDatasetH *ds)
{

  gdalcommon_calcstats(*ds);
}

void gdalcalcstatsignore_(GDALDatasetH *ds,float *ignore)
{
  gdalcommon_calcstatsignore(*ds,*ignore);
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
