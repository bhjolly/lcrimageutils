
#include <stdio.h>
#include <stdlib.h>

#include "gdal_priv.h"
#include "gdal_frmts.h"

#include "gdalcommon_cpp.h"

enum EClumpType {
  eBySize, 
  eByID,
  eBoth
};

template<class T>
class CClump
{
public:
  CClump(CMemRaster<T> *indata, T ignoreval=0)
  {
    m_pindata = indata;
    m_ignoreval = ignoreval;
    m_pidClump = NULL;
    m_psizeClump = NULL;
  }
  
  ~CClump()
  {
    delete m_pidClump;
    delete m_psizeClump;
  }
  
  void clump(EClumpType type)
  {
    // tmp pointer to check if done pixel - points to valid output
    CMemRaster<GUInt32> *pOut = NULL;
  
    // allocate arrays
    if( ( type == eBySize ) || ( type == eBoth ) )
    {
      delete m_psizeClump;
      m_psizeClump = new CMemRaster<GUInt32>(m_pindata->getXSize(),m_pindata->getYSize());
      m_psizeClump->setAll(m_ignoreval);
      pOut = m_psizeClump;
    }
    if( ( type == eByID ) || ( type == eBoth ) )
    {
      delete m_pidClump;
      m_pidClump = new CMemRaster<GUInt32>(m_pindata->getXSize(),m_pindata->getYSize());
      m_pidClump->setAll(m_ignoreval);
      pOut = m_pidClump;
    }
    
    // set all to ignore val
    int lastpercent = -1;
    GUInt32 id = 1;
    
    for( int ycount = 0; ycount < m_pindata->getYSize(); ycount++ )
    {
      for( int xcount = 0; xcount < m_pindata->getXSize(); xcount++ )
      {
        if( pOut->getValue(xcount,ycount) == m_ignoreval )  // hasn't already been filled in
        {
          if( m_pindata->getValue(xcount,ycount) != m_ignoreval ) // is something in the input image
          {
            CCoordList totalList;
            clumpFromPoint(xcount,ycount,totalList);
            if( ( type == eBySize ) || ( type == eBoth ) )
            {
              m_psizeClump->setValues(totalList, totalList.size());
            }
            if( ( type == eByID ) || ( type == eBoth ) )
            {
              m_pidClump->setValues(totalList, id);
              id++;
            }
          }
        }
      } 
      int percent = int((float)ycount * 100 / (float)m_pindata->getYSize());
      if( percent != lastpercent )
      {
        fprintf( stderr, "%d%%\r", percent );
        lastpercent = percent;
      } 
    }
  }
  
  void clumpFromPoint( int x, int y, CCoordList &totalList )
  {
	  CCoordList searchList;
	  searchList.addXY(x,y);
    totalList.addXY(x,y);
  
	  // while we are still have pixels to search
	  while(searchList.size() != 0 )
	  {
	    // create a list with all the new coords we have found
	    CCoordList newCoords;
	    for( CCoordList::tItr itr = searchList.begin(); itr != searchList.end(); itr++)
	    {
	      const CCoord &coord = (*itr);
        CCoordList tmp = searchPixel(coord.getX(),coord.getY());
  	    for( CCoordList::tItr itr = tmp.begin(); itr != tmp.end(); itr++)
	      {
	        const CCoord coord = (*itr);
          if( ! totalList.exists(coord) )
          {
            newCoords.addCoord(coord);
            totalList.addCoord(coord);
          }
        }
	    } 
		
	    // look throught these new coords
	    searchList = newCoords;
	  }
  }

  CCoordList searchPixel( int x, int y)
	{
	  CCoordList coordsToCheck;

	  // Find all all the points we can search from here.
	  if( x > 0 )
	  {
	    coordsToCheck.addXY(x-1,y);
	    if( y > 0 )
	    {
	      coordsToCheck.addXY(x-1,y-1);
	    }
	    if( y < m_pindata->getYSize() - 1 )
	    {
  	    coordsToCheck.addXY(x-1,y+1);
	    }
	  }
	  if( x < m_pindata->getXSize() - 1 )
	  {
	    coordsToCheck.addXY(x+1,y);
	    if( y > 0 )
	    {
	      coordsToCheck.addXY(x+1,y-1);
	    }
	    if( y < m_pindata->getYSize() - 1 )
	    {
	      coordsToCheck.addXY(x+1,y+1);
	    }
	  }
  	if( y >  0 )
	  {
    	coordsToCheck.addXY(x,y-1);
  	}  
	  if( y < m_pindata->getYSize() - 1 )
	  {
  	  coordsToCheck.addXY(x,y+1);
	  }

    CCoordList newlist;
	  for( CCoordList::tItr itr = coordsToCheck.begin(); itr != coordsToCheck.end(); itr++ )
	  {
  	  const CCoord &coord = (*itr);
      // is it part of the clump?
	    if( m_pindata->getValue(coord) != m_ignoreval )
      {
        newlist.addCoord(coord);
      }
	  }
  
  	return newlist;
	}
  
  CMemRaster<GUInt32>* getIDs()
  {
    return m_pidClump;
  }

  CMemRaster<GUInt32>* getSizes()
  {
    return m_psizeClump;
  }

protected:
  CMemRaster<T> *m_pindata;
  CMemRaster<GUInt32> *m_pidClump;
  CMemRaster<GUInt32> *m_psizeClump;
  T m_ignoreval;
};

void printUsage()
{
  fprintf( stderr, "usage: infile outfile --size|--id|--both\n" );
  exit(1);
}

int main(int argc,char *argv[])
{

  if( argc != 4 )
  {
    printUsage();
  }

  char *pszInFile = argv[1];
  char *pszOutFile = argv[2];
  char *pszType = argv[3];
  
  EClumpType type = eBySize;
  if( strcasecmp( pszType, "--size" ) == 0 )
  {
    type = eBySize;
  }
  else if( strcasecmp( pszType, "--id" ) == 0 )
  {  
    type = eByID;
  }
  else if( strcasecmp( pszType, "--both" ) == 0 )
  {  
    type = eBoth;
  }
  else
  {
    printUsage();
  }

  GDALAllRegister();

  GDALDataset *pInDataset = (GDALDataset *) GDALOpen( pszInFile, GA_ReadOnly );

  GDALRasterBand *pInBand = pInDataset->GetRasterBand(1);

  int xsize = pInBand->GetXSize();
  int ysize = pInBand->GetYSize();
  
  CMemRaster<GByte> inRaster(xsize,ysize);
  pInBand->RasterIO( GF_Read, 0, 0, xsize, ysize, inRaster.getRaster(), 
                        xsize, ysize, GDT_Byte, 0, 0 );
  
  CClump<GByte> clump(&inRaster);
  
  clump.clump( type );
  
  // Write out the output raster
  int nBands = 1;
  if( type == eBoth )
  {
    nBands = 2;
  }
  GDALDataset *pOutDataset = (GDALDataset *)gdalcommon_newfile_templ((GDALDatasetH)pInDataset,pszOutFile,0,0,nBands,GDT_UInt32);
  if( pOutDataset == NULL )
  {
    fprintf( stderr, "Cannot open %s\n", pszOutFile );
    exit(1);
  }
  
  delete pInDataset;
  pInDataset = NULL;

  if( type == eByID )
  {
    GDALRasterBand *pOutBand = pOutDataset->GetRasterBand(1);
    pOutBand->RasterIO(GF_Write,0,0,xsize,ysize,clump.getIDs()->getRaster(),xsize,ysize,GDT_UInt32,0,0);
    pOutBand->SetDescription("id");
  }
  else if( type == eBySize )
  {
    GDALRasterBand *pOutBand = pOutDataset->GetRasterBand(1);
    pOutBand->RasterIO(GF_Write,0,0,xsize,ysize,clump.getSizes()->getRaster(),xsize,ysize,GDT_UInt32,0,0);
    pOutBand->SetDescription("size");
  }
  else // eAll
  {
    GDALRasterBand *pOutBand = pOutDataset->GetRasterBand(1);
    pOutBand->RasterIO(GF_Write,0,0,xsize,ysize,clump.getIDs()->getRaster(),xsize,ysize,GDT_UInt32,0,0);
    pOutBand->SetDescription("id");
    
    pOutBand = pOutDataset->GetRasterBand(2);
    pOutBand->RasterIO(GF_Write,0,0,xsize,ysize,clump.getSizes()->getRaster(),xsize,ysize,GDT_UInt32,0,0);
    pOutBand->SetDescription("size");
  }
  

  delete pOutDataset;
  pOutDataset = NULL;

	fprintf( stderr, "Calculating Statistics...\n" );

  gdalcommon_calcstats( pszOutFile );

  return 0;
}
