
#include "gdal_priv.h"
#include "gdal_frmts.h"

#include "gdalcommon_cpp.h"

class CRegionLabelling
{
public:

  static int label(CMemRaster<GByte> &inoutData, bool bCompact);
  
  //typedef std::map<int, float> tID2Value;

  //static tID2Value findMeans(CMemRaster<GByte> &inData,CMemRaster<GByte> &inRegions);

};
