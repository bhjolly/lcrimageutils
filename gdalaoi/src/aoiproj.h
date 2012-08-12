
#ifndef AOIPROJ_H
#define AOIPROJ_H

// Routines for dealing with projections and
// transform polynomials
// Adapted from HFA driver
OGRSpatialReference* CreateSpatialReference( HFAEntry* pAOInode );
const Eprj_ProParameters *AOIGetProParameters( HFAEntry *poAntNode );
const Eprj_Datum *AOIGetDatum( HFAEntry *poAntNode );
const Eprj_MapInfo *AOIGetMapInfo( HFAEntry *poAntNode );

void ApplyXformPolynomial( Efga_Polynomial *pPoly, double *pdfX, double *pdfY );
void ReadXformPolynomial( HFAEntry *pElement, Efga_Polynomial *pPoly );

#endif // AOIPROJ_H
