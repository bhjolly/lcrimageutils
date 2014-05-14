#include "ginfo.h"
#include "gdal.h"
#include "fparse.h"

#ifndef GLAYER_H
#define GLAYER_H

#define pGlayer(glayer,x,y) ((char*)glayer->line[y] + (x)*glayer->nbpp)
#define Glayer(glayer,type,x,y) (*(type *)pGlayer(glayer,x,y))

typedef struct     /* imagery layer */
{
  GDALDatasetH    hDataset;
  GDALRasterBandH hBand;

  Eprj_MapInfo *info;

  char *pszProjection;
  char **papszMetadata;

  GDALDataType pixeltype;

  Eimg_LayerType layertype;

  Esta_Statistics *stats; 

  void *data;   /* pointer to actual image data */

  void **line;  /* pointers to start of image lines */

  long width;
  long height;

  int blockwidth;
  int blockheight;

  /* Raster Colortable */

  GDALColorTableH hCT;

  /* Raster Attribute Table */

  GDALRasterAttributeTableH hRAT;
  
  /* Map information */

  double GeoTransform[6];

  /* GeoTransform - coefficients for transforming between pixel/line (i,j) raster space,
   * and projection coordinates (E,N) space
   *
   * GeoTransform[0]  top left x 
   * GeoTransform[1]  w-e pixel resolution nfo->proName.data
   * GeoTransform[2]  rotation, 0 if image is "north up" 
   * GeoTransform[3]  top left y 
   * GeoTransform[4]  rotation, 0 if image is "north up" 
   * GeoTransform[5]  n-s pixel resolution 
   *
   * E = GeoTransform[0] + i*GeoTransform[1] + j*GeoTransform[2];
   * N = GeoTransform[3] + i*GeoTransform[4] + j*GeoTransform[5];
   * 
   * In a north up image, GeoTransform[1] is the pixel width, and GeoTransform[5] is the pixel height.
   * The upper left corner of the upper left pixel is at position (GeoTransform[0],GeoTransform[3]).
   */

  int nbpp;     /* number of bytes per pixel  */
  int nbpl;     /* number of bytes per line */

  double pw;    /* pixel width  (map units) */
  double ph;    /* pixel height             */
  double pa;    /* pixel area               */

}
Glayer;

#define GlayerReadSingle(fname) (GlayerReadLayer(fname, 1))
#define GlayerOpenSingle(fname) (GlayerOpenLayer(fname, 1))
#define GlayerWriteSingle(glayer, fname) (GlayerWriteLayer(glayer, fname, 1))

Glayer *
GlayerOpenLayer(char *fname, int band);

Glayer *
GlayerReadLayer(char *fname, int band);

void
GlayerWriteLayer(Glayer *glayer, char *fname, int band);

void CreatePyramids(char *fname, char *pszType);

Glayer *
GlayerCreate(long width, long height, GDALDataType pixeltype, Eprj_MapInfo *info, char *pszProjection);

Glayer *
GlayerCreateNoData(long width, long height, GDALDataType pixeltype, Eprj_MapInfo *info, char *pszProjection);

void GlayerCreateData(Glayer *glayer, long width, long height);

void GlayerFreeData(Glayer *glayer);

void GlayerFree(Glayer **glayer);

void GlayerCheckType(Glayer *glayer, GDALDataType pixeltype);

int  setnbpp(GDALDataType pixeltype);

Eprj_MapInfo *
CreateMapInfo();

Eprj_MapInfo *
CopyMapInfo(Eprj_MapInfo *dest, Eprj_MapInfo *src);

Eprj_MapInfo *
GeoTransformToMapInfo(Eprj_MapInfo *info, double *Geotransform, long width, long height);

double *
MapInfoToGeoTransform(double *GeoTransform, Eprj_MapInfo *info);

void
GlayerPrintXY(Glayer *glayer, long x, long y);

#endif
