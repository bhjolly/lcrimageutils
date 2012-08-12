
#ifndef AOILAYER_H
#define AOILAYER_H

#include <ogrsf_frmts.h>
#include "hfa_p.h"

// Classes for representing layers in an AOI file
// We have 3 layers - one for Polygons, one for lines
// and one for points. 
// These are represented by different classes all
// derived from OGRAOILayer which has common functionality
class OGRAOILayer : public OGRLayer
{
protected:
    OGRFeatureDefn         *m_poFeatureDefn;
    OGRSpatialReference    *m_poSpatialRef;

    HFAEntry               *m_pAOInode;
    HFAEntry               *m_pAOIObject; // pointer to current Eaoi_AoiObjectType

    int                     m_nNextFID;
    int                     m_bEnd;

    HFAEntry*           GetNextAOIObject();
    HFAEntry*           GetInfoFromAOIObject(HFAEntry *pAOIObject, 
                            const char **ppszName, const char **ppszDescription );

    void HandleChildFeatures(HFAEntry *pNode, HFAEntry *pParent, OGRGeometryCollection *pCollection);
    OGRGeometry *       HandlePolygon( HFAEntry *pInfo, Efga_Polynomial *pPoly );
    OGRGeometry *       HandleRectangle( HFAEntry *pInfo, Efga_Polynomial *pPoly );
    OGRGeometry *       HandleEllipse( HFAEntry *pInfo, Efga_Polynomial *pPoly );
    OGRGeometry *       HandleLine( HFAEntry *pInfo, Efga_Polynomial *pPoly );
    OGRGeometry *       HandlePoint( HFAEntry *pInfo, Efga_Polynomial *pPoly );

  public:
    OGRAOILayer( HFAEntry *pAOInode, const char *pszBasename );
   ~OGRAOILayer();

    void                ResetReading();
    OGRFeature *        GetNextFeature();

    OGRFeatureDefn *    GetLayerDefn() { return m_poFeatureDefn; }
    OGRSpatialReference * GetSpatialRef();

    int                 TestCapability( const char * ) { return FALSE; }
};

#endif // AOILAYER_H
