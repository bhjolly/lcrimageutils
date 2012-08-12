
#ifndef AOIDATASOURCE_H
#define AOIDATASOURCE_H

#include <ogrsf_frmts.h>
#include "aoilayer.h"

// Data source class for AOI files
class OGRAOIDataSource : public OGRDataSource
{
    char                *m_pszName;
    int                  m_nLayers;
    
    OGRAOILayer         *m_poLayer;
    HFAInfo_t	        *m_psInfo;

  public:
                        OGRAOIDataSource();
                        ~OGRAOIDataSource();

    int                 Open( const char * pszFilename, int bUpdate );
    
    const char          *GetName() { return m_pszName; }

    int                 GetLayerCount() { return m_nLayers; }
    OGRLayer            *GetLayer( int );

    int                 TestCapability( const char * ) { return FALSE; }

};

#endif // AOIDATASOURCE_H
