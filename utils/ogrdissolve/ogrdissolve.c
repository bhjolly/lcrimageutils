/*
Program to dissolve all the polygons in an input file and
create a new output file. Uses the GEOS cascaded union function.

Need to add support for multiple input files and dissolving on attributes.

If adding support for multiple input files, I would suggest re-writing
this in C++ and using an STL vector for holding the geometries so you can
loop through each file and add the geoms in without having to allocate
enough space first.

Other option is to re-write this in Python, but it seems that the GEOS
Python bindings no longer work and Shapely seems plain weird. 

*/

#include <stdio.h>
#include <stdarg.h>

#include "ogr_api.h"
#include "geos_c.h"

/* Handler that gets called if there is an ERROR in GEOS */
void geosHandler(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    fprintf( stderr, "Message from GEOS:\n" );
    vfprintf( stderr, fmt, ap );
    
    va_end(ap);
}

/* Creates a GEOS collection from an OGR data source */
GEOSGeometry *collectionFromOGR(const char *pszFilename)
{
    printf( "Reading file...\n" );

    /* Open the input OGR dataset */
    OGRDataSourceH hDS = OGROpen( pszFilename, FALSE, NULL );
    if( hDS == NULL )
    {
        fprintf( stderr, "Unable to open %s\n", pszFilename );
        exit(1);
    }
    
    /* Get this first layer. This is OK for shape files. Not sure about others... */
    OGRLayerH hLayer = OGR_DS_GetLayer( hDS, 0 );
    OGR_L_ResetReading(hLayer);
    
    /* Get the number of features in this dataset so we now how much to malloc */
    int nFeatures = OGR_L_GetFeatureCount(hLayer,TRUE);
    
    /* Create our list of GEOSGeometry*'s that we create a collection from later */
    GEOSGeometry **ppGeomList = (GEOSGeometry**)calloc( nFeatures, sizeof(GEOSGeometry*) );

    /* Create the GEOS WKB reader */
    GEOSWKBReader *GeosReader = GEOSWKBReader_create();

    /* loop thru each feature in the file */
    int nCount = 0;
    OGRFeatureH hFeature;
    int nLastPercent = -1;
    while( (hFeature = OGR_L_GetNextFeature(hLayer)) != NULL )
    {
        /* Update percent */
        int nPercent = (int)( (double)nCount / (double)nFeatures * 100.0 );
        if( nPercent != nLastPercent )
        {
          fprintf( stderr, "%d%%\r", nPercent );
          nLastPercent = nPercent;
        }

        /* Get the geometry */
        OGRGeometryH hOGRGeom = OGR_F_GetGeometryRef(hFeature);
        if( hOGRGeom != NULL )
        {
            /* Is it OK? */
            if( OGR_G_IsValid(hOGRGeom) )
            {
                /* Get the size that the WKB for this Geometry is going to be */
                int nWKBSize = OGR_G_WkbSize( hOGRGeom );
                
                /*fprintf( stderr, "%d %d %d %d %d %d %d\n", nWKBSize, nCount, nFeatures, OGR_G_IsEmpty(hOGRGeom), OGR_G_IsSimple(hOGRGeom), OGR_G_IsValid(hOGRGeom), OGR_G_IsRing(hOGRGeom) );*/
            
                /* Allocate memory for the WKB */
                /* If I was smarter I would resuse the bufffer */
                unsigned char *pWKBBuffer = (unsigned char*)malloc( nWKBSize );
                
                /* Get the Geometry as a WKB */
                OGR_G_ExportToWkb( hOGRGeom, wkbNDR, pWKBBuffer );
                
                /* Read in the WKB as a GEOSGeometry using the reader */ 
                ppGeomList[nCount] = GEOSWKBReader_read( GeosReader, pWKBBuffer, nWKBSize );
                
                /* Free the buffer */
                free( pWKBBuffer );
                
                /* Increment our counter */
                nCount++;
            }
        }
    
        /* Destroy the feature object */
        OGR_F_Destroy( hFeature );
    }
    /* Close the OGR datasource */
    OGR_DS_Destroy( hDS );

    /* We have finished with the reader also */
    GEOSWKBReader_destroy( GeosReader );
    
    /* Create a GEOS collection that holds all output data */  
    GEOSGeometry* pGeoCollection = GEOSGeom_createCollection( GEOS_MULTIPOLYGON, ppGeomList, nCount );

    /* Free our list of geometries that was used to make a collection */    
    free( ppGeomList );
    
    return pGeoCollection;
}

/* Writes out the result to a shapefile */
void writeCollectionToOGR( const char *pszFilename, GEOSGeometry *pGEOSCollection )
{
    /* Get the driver for a shapefile */
    OGRSFDriverH hDriver = OGRGetDriverByName("ESRI Shapefile");
    
    /* Create the OGR datasource */
    OGRDataSourceH hDS = OGR_Dr_CreateDataSource( hDriver, pszFilename, NULL );
    if( hDS == NULL )
    {
        fprintf( stderr, "Unable to create %s\n", pszFilename );
    }

    /* Create our layer - there is only one */
    OGRLayerH hLayer = OGR_DS_CreateLayer( hDS, "out", NULL, wkbPolygon, NULL );
    
    /* Create a GEOSWriter */
    GEOSWKBWriter *geosWriter = GEOSWKBWriter_create();
    
    /* Set byte order to Intel */
    GEOSWKBWriter_setByteOrder( geosWriter, GEOS_WKB_NDR );
    
    /* Loop thru eachGeometry */
    size_t nSize = 0;
    int nCount;
    for( nCount = 0; nCount < GEOSGetNumGeometries(pGEOSCollection); nCount++ )
    {
        /* Get the current Geometry out of the collection */
        const GEOSGeometry *pOutGeom = GEOSGetGeometryN( pGEOSCollection, nCount );
        
        /* Write it to WKB */
        unsigned char *pWKBBuffer = GEOSWKBWriter_write( geosWriter, pOutGeom, &nSize );
        
        /* Create an empty OGR Geometry */
        OGRGeometryH hOGRGeom = OGR_G_CreateGeometry( wkbPolygon );
        
        /* make the OGR Geometry object from the WKB */
        OGR_G_ImportFromWkb( hOGRGeom, pWKBBuffer, nSize );
        
        /* Create a new feature */
        OGRFeatureH hFeature = OGR_F_Create( OGR_L_GetLayerDefn( hLayer ) );

        /* set the geometry for the feature */
        OGR_F_SetGeometry( hFeature, hOGRGeom ); 
        
        /* Destroy the OGR geom */
        OGR_G_DestroyGeometry( hOGRGeom );

        /* Add the feature to the layer */
        OGR_L_CreateFeature( hLayer, hFeature );

        /* destory the feature */
        OGR_F_Destroy( hFeature );

        /* Free the WKB */
        GEOSFree( pWKBBuffer );
    }
    
    /* Finished with the writer */
    GEOSWKBWriter_destroy( geosWriter );
    
    /* Close the datasource */
    OGR_DS_Destroy( hDS );
}

int main( int argc, char *argv[] )
{
    if( argc != 3 )
    {
        fprintf( stderr, "usage: ogrdissolve infile outshp\n" );
        exit(1);
    }

    /* Initialise OGR */
    OGRRegisterAll();
    
    /* Initialise GEOS */
    initGEOS(geosHandler, geosHandler);

    /* Read in the Geometry's from the input file */
    GEOSGeometry *pGeoCollection = collectionFromOGR( argv[1] );
    
    /* Now do the union */
    printf( "Doing dissolve...\n" );
    GEOSGeometry* pGeoCascaded = GEOSUnionCascaded( pGeoCollection );
    
    printf( "%d -> %d\n", GEOSGetNumGeometries(pGeoCollection), GEOSGetNumGeometries(pGeoCascaded) );
    
    printf( "Writing output...\n" );
    writeCollectionToOGR( argv[2], pGeoCascaded );
    
    /* Destroy our Geometry collections */ 
    /* Not sure if we should be looping thru.... */
    GEOSGeom_destroy( pGeoCollection );
    GEOSGeom_destroy( pGeoCascaded );

    /* close down GEOS */
    finishGEOS();

    return 0;
}
