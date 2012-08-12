
#ifndef AOIDRIVER_H
#define AOIDRIVER_H

#include <ogrsf_frmts.h>

// Driver class for the AOI reader
class OGRAOIDriver: public OGRSFDriver
{
public:
    ~OGRAOIDriver();

    const char*     GetName();
    OGRDataSource*  Open( const char *pszFileame, int bUpdate );
    OGRErr          DeleteDataSource( const char *pszName );
    int             TestCapability( const char *pszCap );
};

#endif // AOIDRIVER_H
