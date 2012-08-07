#include "glayer.h"
#include "gutils.h"
#include "cpl_string.h"

Glayer *
GlayerOpenLayer(char *fname, int band)
{
  Glayer *glayer = (Glayer *)calloc(1, sizeof(Glayer));

  glayer->hDataset = GDALOpen(fname, GA_ReadOnly);

  if(glayer->hDataset == NULL)
    errexit(outerr, "error opening %s", fname);

  glayer->hBand = GDALGetRasterBand(glayer->hDataset, band);
  
  glayer->pixeltype = GDALGetRasterDataType(glayer->hBand);  

  glayer->width  = GDALGetRasterBandXSize(glayer->hBand);
  glayer->height = GDALGetRasterBandYSize(glayer->hBand);

  GDALGetBlockSize(glayer->hBand, &glayer->blockwidth, &glayer->blockheight);

  glayer->nbpp = setnbpp(glayer->pixeltype);
  glayer->nbpl = glayer->width*glayer->nbpp;

  if(GDALGetGeoTransform(glayer->hDataset, glayer->GeoTransform) == CE_None)
    {
      glayer->pszProjection = strdup(GDALGetProjectionRef(glayer->hDataset));

      glayer->info = GeoTransformToMapInfo(glayer->info, glayer->GeoTransform, glayer->width, glayer->height);
 
      glayer->pw = glayer->info->pixelSize->width;
      glayer->ph = glayer->info->pixelSize->height;
      glayer->pa = glayer->pw*glayer->ph;
    }

  glayer->hCT = GDALGetRasterColorTable(glayer->hBand);

  glayer->papszMetadata = GDALGetMetadata(glayer->hBand, NULL);
  glayer->hRAT = GDALGetDefaultRAT(glayer->hBand);

#ifdef VERBOSE
  if(CSLCount(glayer->papszMetadata) > 0)
    {
      fprintf(stdout,"Metadata:\n");
      
      for(j = 0; glayer->papszMetadata[j] != NULL; j++)
    fprintf(stdout,"  %s\n", glayer->papszMetadata[j]);
    }

  if(glayer->hRAT)
    GDALRATDumpReadable(glayer->hRAT, NULL);
#endif
 
  return(glayer);
}

Glayer *
GlayerReadLayer(char *fname, int band)
{
  long j;
  double percent_per_line;
  double percent;
  Glayer *glayer;
  
  glayer = GlayerOpenLayer(fname, band);

  GlayerCreateData(glayer, glayer->width, glayer->height);

  percent_per_line = 100.0/glayer->height;
  percent = 0.0;

  jobstate("Reading %s [Layer %d]...", fname, band);
  jobprog(0);

  for(j = 0; j < glayer->height; j++)
    {
      if(floor(j*percent_per_line) > percent)
    {
      percent = j*percent_per_line;
      jobprog(percent);
    }

      GDALRasterIO(glayer->hBand, GF_Read, 0, j, glayer->width, 1, glayer->line[j], glayer->width, 1, glayer->pixeltype, 0, 0);
    }
 
  jobprog(100);
  
  GDALClose(glayer->hDataset);
  
  glayer->hBand = NULL;
  
  return(glayer);
}

void 
QuietErrorHandler(CPLErr eErrClass, int err_no, const char *msg)
{
  return;
}

void
GlayerWriteLayer(Glayer *glayer, char *fname, int band)
{
  char **pszStringList = NULL;
  long j;
  double percent_per_line;
  double percent;
  char driver[80];
          
  if(!strcmp(fextn(fname),".img"))
    {
      sprintf(driver, "HFA");
      pszStringList = (char **)CSLAddString(pszStringList, "COMPRESS=TRUE");
    }
  else if(!strcmp(fextn(fname),".tif"))
    sprintf(driver, "GTiff");
  else
    errexit(outerr,"unsupported file extension");

  /* First try opening it (installing our own error handler first so no error printed) */
  CPLSetErrorHandler(QuietErrorHandler);

  glayer->hDataset = GDALOpen(fname, GA_Update);

  CPLSetErrorHandler(NULL);

  if(glayer->hDataset == NULL)
  {  
    /* Can't open, try creating it */
    jobstate("Creating %s... (%s)", fname, driver);

    glayer->hDataset = GDALCreate(GDALGetDriverByName(driver), fname, glayer->width, glayer->height, band, glayer->pixeltype, pszStringList);

    if(glayer->hDataset == NULL)
      errexit(outerr,"GDALCreate failed");

    GDALSetGeoTransform(glayer->hDataset, glayer->GeoTransform);
    GDALSetProjection(glayer->hDataset, glayer->pszProjection);
  }

  glayer->hBand = GDALGetRasterBand(glayer->hDataset, band);
  if(glayer->hBand == NULL)
    errexit(outerr,"Unable to get band");

  percent_per_line = 100.0/glayer->height;
  percent = 0.0;

  jobstate("Writing Layer...");
  jobprog(0);

  for(j = 0; j < glayer->height; j++)
    {
      if(floor(j*percent_per_line) > percent)
    {
      percent = j*percent_per_line;
      jobprog(percent);
    }

      GDALRasterIO(glayer->hBand, GF_Write, 0, j, glayer->width, 1, glayer->line[j], glayer->width, 1, glayer->pixeltype, 0, 0);
    }
 
  jobprog(100);

  GDALClose(glayer->hDataset);

  CSLDestroy(pszStringList);   
}

void CreatePyramids(char *fname, char *pszType)
{
  GDALDatasetH hDataset;
  GDALRasterBandH hBand;
  int nLevels[] = { 4, 8, 16, 32, 64, 128 };  /* These are the pyramid levels Imagine seems to use */

  hDataset = GDALOpen(fname, GA_Update);

  /* Need to find out if we are thematic or continuous */
  hBand = GDALGetRasterBand(hDataset, 1);
 
  if(pszType == NULL)
    {
      if(strcmp(GDALGetMetadataItem(hBand, "LAYER_TYPE", ""), "athematic") == 0)
    pszType = "AVERAGE";
      else
    pszType = "NEAREST";
    }
 
  GDALBuildOverviews(hDataset, pszType, sizeof(nLevels)/sizeof(int), nLevels, 0, NULL, GDALTermProgress, NULL);

  GDALClose(hDataset);
}

Glayer *
GlayerCreate(long width, long height, GDALDataType pixeltype, Eprj_MapInfo *info, char *pszProjection)
{
  Glayer *glayer;
  
  glayer = GlayerCreateNoData(width, height, pixeltype, info, pszProjection);

  GlayerCreateData(glayer, width, height);

  return(glayer);
}

Glayer *
GlayerCreateNoData(long width, long height, GDALDataType pixeltype, Eprj_MapInfo *info, char *pszProjection)
{
  Glayer *glayer = (Glayer *)calloc(1,sizeof(Glayer));
  
  glayer->width = width;
  glayer->height = height;
  glayer->pixeltype = pixeltype;
  
  glayer->nbpp = setnbpp(glayer->pixeltype);

  if(info)
    {
      glayer->info = CopyMapInfo(glayer->info, info);

      glayer->pw = info->pixelSize->width;
      glayer->ph = info->pixelSize->height;
      glayer->pa = glayer->pw*glayer->ph;

      MapInfoToGeoTransform(glayer->GeoTransform, info);

      glayer->pszProjection = strdup(pszProjection);
    }
  
  return(glayer);
}

void
GlayerCreateData(Glayer *glayer, long width, long height)
{
  long j;

  if(glayer->data) free(glayer->data);

  if(!glayer->nbpp)
    glayer->nbpp = setnbpp(glayer->pixeltype);

  glayer->nbpl = width*glayer->nbpp;

  if(glayer->data) free(glayer->data);

  /* allocate data memory */

  glayer->data = malloc((size_t)(height*glayer->nbpl));
      
  if(!glayer->data)
    errexit(outerr,"error allocating image memory");
  
  glayer->data = memset(glayer->data, 0, (size_t)(height*glayer->nbpl));
  
  glayer->line = (void **)calloc(height, sizeof(void *));

  for(j = 0; j < height; j++)
    glayer->line[j] = glayer->data + j*glayer->nbpl;
}

void
GlayerFreeData(Glayer *glayer)
{
  if(glayer->data)
    free(glayer->data);
  
  glayer->data = NULL;

  if(glayer->line)
    free(glayer->line);
  
  glayer->line = NULL;
  
  glayer->nbpp = 0;
  glayer->nbpl = 0;
}

void
GlayerFree(Glayer **glayer)
{
  GlayerFreeData(*glayer);

  free((*glayer)->info);
  free((*glayer)->pszProjection);

  free(*glayer);

  *glayer = NULL;
}

void
GlayerCheckType(Glayer *glayer, GDALDataType pixeltype)
{
  if(glayer->pixeltype != pixeltype)
    errexit(outerr,"expecting %s data [%s]", GDALGetDataTypeName(pixeltype), GDALGetDataTypeName(glayer->pixeltype));
}

int
setnbpp(GDALDataType pixeltype)
{
  int nbpp=0;
  
  switch(pixeltype)
    {
    case GDT_Byte:
      nbpp = sizeof(GByte);
      break;
    case GDT_UInt16:
      nbpp = sizeof(GUInt16);
     break;
    case GDT_Int16:
      nbpp = sizeof(GInt16);
      break;
    case GDT_UInt32:
      nbpp = sizeof(GUInt32);
      break;
    case GDT_Int32:
      nbpp = sizeof(GInt32);
      break;
    case GDT_Float32:
      nbpp = sizeof(float);
      break;
    case GDT_Float64:
      nbpp = sizeof(double);
      break;
    default:
      errexit(outerr,"datatypes greater than Float64 not yet supported");
      break;
    }
  
  return(nbpp);
}

Eprj_MapInfo *
CreateMapInfo()
{
  Eprj_MapInfo *info;

  info = (Eprj_MapInfo *)calloc(1, sizeof(Eprj_MapInfo));

  info->upperLeftCenter  = (Eprj_Coordinate *)calloc(1, sizeof(Eprj_Coordinate));
  info->lowerRightCenter = (Eprj_Coordinate *)calloc(1, sizeof(Eprj_Coordinate));
  info->pixelSize        = (Eprj_Size *)calloc(1, sizeof(Eprj_Size));
  
  return(info);
}

Eprj_MapInfo *
CopyMapInfo(Eprj_MapInfo *dest, Eprj_MapInfo *src)
{
  if(!dest)
    dest = CreateMapInfo();

  if(!src)
    errexit(outerr, "undefined source MapInfo");
  
  dest->upperLeftCenter->x  = src->upperLeftCenter->x;
  dest->upperLeftCenter->y  = src->upperLeftCenter->y;

  dest->lowerRightCenter->x = src->lowerRightCenter->x;
  dest->lowerRightCenter->y = src->lowerRightCenter->y;

  dest->pixelSize->width    = src->pixelSize->width;
  dest->pixelSize->height   = src->pixelSize->height;    
 
  return(dest);
}

Eprj_MapInfo *
GeoTransformToMapInfo(Eprj_MapInfo *info, double *GeoTransform, long width, long height)
{
  if(!GeoTransform)
    errexit(outerr, "undefined GeoTransform");
  
  if(GeoTransform[2] != 0 || GeoTransform[4] != 0)
    errexit(outerr, "expecting north up imagery only");

  if(!info)
    info = CreateMapInfo();
  
  info->upperLeftCenter->x  = GeoTransform[0] + 0.5*GeoTransform[1];
  info->upperLeftCenter->y  = GeoTransform[3] + 0.5*GeoTransform[5];

  info->pixelSize->width    = fabs(GeoTransform[1]);
  info->pixelSize->height   = fabs(GeoTransform[5]);

  info->lowerRightCenter->x = info->upperLeftCenter->x + (width - 1)*info->pixelSize->width;
  info->lowerRightCenter->y = info->upperLeftCenter->y - (height - 1)*info->pixelSize->height;

  return(info);
}

double *
MapInfoToGeoTransform(double *GeoTransform, Eprj_MapInfo *info)
{
  if(!info)
    errexit(outerr, "undefined MapInfo");
    
  if(!GeoTransform)
    GeoTransform = (double *)calloc(6, sizeof(double));

  GeoTransform[0] = info->upperLeftCenter->x - 0.5*info->pixelSize->width;
  GeoTransform[1] = info->pixelSize->width;
  GeoTransform[3] = info->upperLeftCenter->y + 0.5*info->pixelSize->width;
  GeoTransform[5] = -info->pixelSize->height;

  return(GeoTransform);  
}

void
GlayerPrintXY(Glayer *glayer, long x, long y)
{
  switch(glayer->pixeltype)
    {
    case GDT_Byte:
      fprintf(stdout,"(%ld,%ld) -> %d\n", x, y, Glayer(glayer,GByte,x,y));
      break;
    case GDT_UInt16:
      fprintf(stdout,"(%ld,%ld) -> %d\n", x, y, Glayer(glayer,GUInt16,x,y));
     break;
    case GDT_Int16:
      fprintf(stdout,"(%ld,%ld) -> %d\n", x, y, Glayer(glayer,GInt16,x,y));
      break;
    case GDT_UInt32:
      fprintf(stdout,"(%ld,%ld) -> %d\n", x, y, (int)Glayer(glayer,GUInt32,x,y));
      break;
    case GDT_Int32:
      fprintf(stdout,"(%ld,%ld) -> %d\n", x, y, (int)Glayer(glayer,GInt32,x,y));
      break;
    case GDT_Float32:
      fprintf(stdout,"(%ld,%ld) -> %f\n", x, y, Glayer(glayer,float,x,y));
      break;
    case GDT_Float64:
      fprintf(stdout,"(%ld,%ld) -> %f\n", x, y, Glayer(glayer,double,x,y));
      break;
    default:
      errexit(outerr,"datatypes greater than Float64 not yet supported");
      break;
    }
}
