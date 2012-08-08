#ifndef GDALCOMMON_H
#define GDALCOMMON_H

CPL_C_START

/* coordinate helper functions */
CPL_DLL void gdalcommon_wld2pix( double *adfGeoTransform, double dGeoX, double dGeoY, int *nX, int *nY );
CPL_DLL void gdalcommon_pix2wld( double *adfGeoTransform, int nX, int nY, double *dGeoX, double *dGeoY );

/* Creating images */
CPL_DLL char** gdalcommon_create_options();
CPL_DLL GDALDatasetH gdalcommon_new(const char *szName, int xsize, int ysize, int bands, GDALDataType type);
CPL_DLL GDALDatasetH gdalcommon_newfile_templ( GDALDatasetH hTempl, const char *szName, int xsize, int ysize, int bands, GDALDataType type );

CPL_DLL GDALDatasetH gdalcommon_newimg( const char *pszInName, int nXSize, int nYSize, int nBands, 
                                GDALDataType eBandType, double dTLX, double dTLY, double dXpix, double dYPix, int nZone );

CPL_DLL void gdalcommon_calcstats( GDALDatasetH hDataset );
CPL_DLL void gdalcommon_calcstatsignore( GDALDatasetH hDataset, double dVal );

CPL_C_END

#endif //GDALCOMMON_H
