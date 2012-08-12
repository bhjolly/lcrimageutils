
#include "aoidriver.h"
#include "aoidatasource.h"

// see http://www.gdal.org/ogr/ogr_drivertut.html

// Must be C callable
CPL_C_START
    void CPL_DLL RegisterOGRAOI();
CPL_C_END

void RegisterOGRAOI()
{
    OGRSFDriverRegistrar::GetRegistrar()->RegisterDriver( new OGRAOIDriver() );
}

OGRAOIDriver::~OGRAOIDriver()
{
}

// Returns the name I have given this driver
const char *OGRAOIDriver::GetName()
{
    return "ERDAS Imagine AOI";
}

/* Try opening the datasource as an AOI */
/* If successful, return a pointer to a new instance of OGRAOIDataSource */
/* otherwise return NULL */
OGRDataSource* OGRAOIDriver::Open( const char *pszFileame, int bUpdate )
{
    OGRAOIDataSource *poDS = new OGRAOIDataSource();

    if( !poDS->Open( pszFileame, bUpdate ) )
    {
        delete poDS;
        return NULL;
    }
    else
    {
        return poDS;
    }
}

/* Test the capabilities of this driver */
/* we can't create datasets, but we can delete them */
int OGRAOIDriver::TestCapability( const char * pszCap )
{
    if( EQUAL(pszCap,ODrCCreateDataSource) )
        return FALSE;
    else if( EQUAL(pszCap,ODrCDeleteDataSource) )
        return TRUE;
    else
        return FALSE;
}

/* Delete the data source - quite easy it is just one file */
int OGRAOIDriver::DeleteDataSource( const char *pszName )
{
    if( VSIUnlink( pszName ) != 0 )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Attempt to unlink %s failed.\n", pszName );
        return OGRERR_FAILURE;
    }
    else
        return OGRERR_NONE;
}
