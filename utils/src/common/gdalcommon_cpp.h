/*
Defines a set of common classes for dealing with images read using GDAL

Sam Gillingham November 2004.

*/

#ifndef __GDALCOMMON_CPP_H___
#define __GDALCOMMON_CPP_H___

#include <vector>
#include <math.h>

extern "C"
{
  #include "gdalcommon.h"
}

#include "histogram.h"

// Represents a single coord in pixel space
class CCoord
{
public:
  CCoord()
  {
    m_x = 0;
    m_y = 0;
  }
  CCoord(const CCoord &coord)
  {
    m_x = coord.getX();
    m_y = coord.getY();
  }
  CCoord(int x, int y)
  {
    m_x = x;
    m_y = y;
  }
  
  int getX() const
  {
    return m_x;
  }
  int getY() const
  {
    return m_y; 
  }
	
	void setX( int x )
	{
		m_x = x;
	}
	
	void setY( int y )
	{
		m_y = y;
	}
	
	void setXY( int x, int y )
	{
		m_x = x;
		m_y = y;
	}
  
  bool equals(const CCoord &coord) const
  {
    return (getX() == coord.getX()) && (getY() == coord.getY());
  }
	
	float getDist(const CCoord &cd) const
	{
		return getDist(cd.getX(), cd.getY());
	}

  float getDist(int x, int y) const
  {
    int xdist = x - getX();
    int ydist = y - getY();
    return sqrt( (float)(xdist * xdist + ydist * ydist) );
  }
  
protected:
  int m_x;
  int m_y;
};

// Class that supports having a value at a coord.
// Aggregates CCoord
template<class T>
class CValueCoord
{
public:
	CValueCoord()
		: m_coord(0,0),
			m_val(0)
	{
	}
  CValueCoord(const CValueCoord &coord)
  {
    m_coord = coord.getCoord();
    m_val = coord.getValue();
  }
  
  CValueCoord( int x, int y, T val )
  {
    m_coord = CCoord(x, y);
    m_val = val;
  }

  CValueCoord( const CCoord &coord, T val )
  {
    m_coord = coord;
    m_val = val;
  }

	CValueCoord<T>& operator=(const CValueCoord<T> &z)
	{
		m_coord = z.getCoord();
		m_val = z.getValue();
		return *this;
	}

  int getX() const
  {
    return m_coord.getX();
  }
  int getY() const
  {
    return m_coord.getY(); 
  }
  
  CCoord getCoord() const
  {
    return m_coord;
  }
  
  T getValue() const
  {
    return  m_val;
  }
  
  bool equals(const CValueCoord &coord) const
  {
    return( m_coord.equals(coord.getCoord) && ( coord.getValue() == m_val ) );
  }

	float getDist(const CCoord &cd) const
	{
		return getDist(cd.getX(), cd.getY());
	}

	float getDist(const CValueCoord<T> &cd) const
	{
		return getDist(cd.getX(), cd.getY());
	}
  
  float getDist(int x, int y) const
  {
    int xdist = x - getX();
    int ydist = y - getY();
    return sqrtf( xdist * xdist + ydist * ydist );
  }
  
protected:
  CCoord m_coord;
  T     m_val;
};

// Represents a list of unique coords
class CCoordList: public std::vector<CCoord>
{
public:
  typedef std::vector<CCoord>::const_iterator tItr;
  
  bool exists(const CCoord &coord) const;
	
	// The next 2 functions check to make sure the element does not exist
  bool addCoord(const CCoord &coord);
  bool addXY(int x, int y)
  {
    CCoord coord(x,y);
    return addCoord(coord); 
  }
	
  bool appendList(const CCoordList &list);
  void drawline2d(const CCoord &c1, const CCoord &c2);
};

// represents a list of CValueCoords
template<class T>
class CValueCoordList : public std::vector<CValueCoord<T> >
{
public:
  CValueCoord<T> findMinimum()
  {
    typename CValueCoordList<T>::const_iterator itr = std::vector<CValueCoord<T> >::begin();
    CValueCoord<T> min = (*itr);
    for( itr; itr != std::vector<CValueCoord<T> >::end(); itr++ )
    {
      if( (*itr).getValue() < min.getValue() )
      {
        min = (*itr);
      }
    } 
    
    return min;
  }
};

// Representation of an in memory raster. Does own malloc and free
template<class T>
class CMemRaster
{
public:
  CMemRaster(int xsize,int ysize)
  {
    m_xsize = xsize;
    m_ysize = ysize;
    m_pRaster = static_cast<T*>(calloc( xsize * ysize, sizeof(T) ));
  }
  
  ~CMemRaster()
  {
    dealloc();
  }
  
  void dealloc()
  {
    free( m_pRaster );
    m_pRaster = NULL;
  }
	
	bool isValidCoord(const CCoord &coord) const
	{
		return ( coord.getX() >= 0 ) && ( coord.getY() >= 0 ) && 
						( coord.getX() < getXSize() ) &&  ( coord.getY() < getYSize() );
	}

  T getValue(int x, int y) const
  {
    return m_pRaster[(y*m_xsize)+x];
  }

  T getValue(const CCoord &coord) const
  {
    return m_pRaster[(coord.getY()*m_xsize)+coord.getX()];
  }

  void setValue(int x, int y, T val)
  {
    m_pRaster[(y*m_xsize)+x] = val;
  }

  void setValue(const CCoord &coord, T val)
  {
    m_pRaster[(coord.getY()*m_xsize)+coord.getX()] = val;
  }
  
  void setValues(const CCoordList &list, T val)
  {
    for( CCoordList::tItr itr = list.begin(); itr != list.end(); itr++ )
    {
      const CCoord coord = (*itr);
      setValue(coord.getX(),coord.getY(),val); 
    }
  }
  
  int getXSize() const
  {
    return m_xsize; 
  }

  int getYSize() const
  {
    return m_ysize; 
  }
  
  T* getRaster()
  {
    return m_pRaster;
  }
  
  void setAll(T val)
  {
    // Treat the array like its one dimensional to improve speed
    int tot = m_xsize * m_ysize;
    for( int count = 0; count < tot; count++ )
    {
       m_pRaster[count] = val;
		}
  }
	
	double getMean() const
	{
		double dTot = 0;
    // Treat the array like its one dimensional to improve speed
    int tot = m_xsize * m_ysize;
    for( int count = 0; count < tot; count++ )
    {
       dTot += m_pRaster[count];
		}
		return dTot / double(tot);
	}
	
	double getMean(T ignoreVal) const
	{
		double dTot = 0;
    // Treat the array like its one dimensional to improve speed
    int tot = m_xsize * m_ysize;
    int total_used = 0;
    for( int count = 0; count < tot; count++ )
    {
      if( m_pRaster[count] != ignoreVal )
      {
        dTot += m_pRaster[count];
        total_used ++;
      }
		}
		return dTot / double(total_used);
	}
  
  double getStdDev(double fMean) const
  {
		double dTot = 0;
    double dVariance = 0;
    // Treat the array like its one dimensional to improve speed
    int tot = m_xsize * m_ysize;
    for( int count = 0; count < tot; count++ )
    {
      double diff = m_pRaster[count] - fMean;
      dTot += diff;
      dVariance += diff * diff;
		}
    dVariance = (dVariance - dTot*dTot/tot)/(tot -1);
		return sqrt(dVariance);
  }
  
  double getStdDev(double fMean,T ignoreVal) const
  {
		double dTot = 0;
    double dVariance = 0;
    // Treat the array like its one dimensional to improve speed
    int tot = m_xsize * m_ysize;
    int total_used = 0;
    for( int count = 0; count < tot; count++ )
    {
      if( m_pRaster[count] != ignoreVal )
      {
        double diff = m_pRaster[count] - fMean;
        dTot += diff;
        dVariance += diff * diff;
        total_used++;
      }
		}
    dVariance = (dVariance - dTot*dTot/total_used)/(total_used -1);
		return sqrt(dVariance);
  }
  
	double multiplyNormalise( double dMean, CMemRaster<T> *pOther, double dMeanOther ) const
	{
		double result = 0;
		if( ( getXSize() != pOther->getXSize() ) || ( getYSize() != pOther->getYSize() ) )
		{
			fprintf( stderr, "CMemRaster::multiply - sizes must match\n");
			exit(1);
		}
		
    // Treat the array like its one dimensional to improve speed
    int tot = m_xsize * m_ysize;
    for( int count = 0; count < tot; count++ )
		{
			result += ( ( m_pRaster[count]  - dMean ) * ( pOther->getRaster()[count] - dMeanOther ) );
		}		
		
		return result;
	}
	
	double multiply( CMemRaster<T> *pOther ) const
	{
		double result = 0;
		if( ( getXSize() != pOther->getXSize() ) || ( getYSize() != pOther->getYSize() ) )
		{
			fprintf( stderr, "CMemRaster::multiply - sizes must match\n");
			exit(1);
		}
		
    // Treat the array like its one dimensional to improve speed
    int tot = m_xsize * m_ysize;
    for( int count = 0; count < tot; count++ )
		{
			result += ( m_pRaster[count] * pOther->getRaster()[count] );
		}		
		
		return result;
	}
  
	CMemRaster<T>* getSquare(const CCoord &tl, const CCoord &br) const
	{
		int nWidth = br.getX() - tl.getX();
		int nHeight = br.getY() - tl.getY();
		CMemRaster<T> *pOut = new CMemRaster<T>(nWidth, nHeight);
		
		for( int ycount = 0; ycount < nHeight; ycount++ )
		{
			for( int xcount = 0; xcount < nWidth; xcount++ )
			{
				pOut->setValue(xcount, ycount, getValue(tl.getX() + xcount, tl.getY() + ycount ) );
			}
		}
		
		return pOut;
	}
	
	void setSquareIgnore( CMemRaster<T> *pSquare, const CCoord &tl, T ignore )
	{
		for( int ycount = 0; ycount < pSquare->getYSize(); ycount++ )
		{
			for( int xcount = 0; xcount < pSquare->getXSize(); xcount++ )
			{
				T val = pSquare->getValue( xcount, ycount );
				if( val != ignore )
				{
					setValue( xcount + tl.getX(), ycount + tl.getY(), val );
				}
			}
		}
	}
	
  void recode(T from, T to)
  {
    // Treat the array like its one dimensional to improve speed
    int tot = m_xsize * m_ysize;
    for( int count = 0; count < tot; count++ )
    {
      if( m_pRaster[count] == from )
      {
        m_pRaster[count] = to; 
      }
    }
  }
	
	void recodeGreater(T greater, T to)
	{
    // Treat the array like its one dimensional to improve speed
    int tot = m_xsize * m_ysize;
    for( int count = 0; count < tot; count++ )
    {
      if( m_pRaster[count] > greater )
      {
        m_pRaster[count] = to; 
      }
    }
	}

	void recodeLess(T less, T to)
	{
    // Treat the array like its one dimensional to improve speed
    int tot = m_xsize * m_ysize;
    for( int count = 0; count < tot; count++ )
    {
      if( m_pRaster[count] < less )
      {
        m_pRaster[count] = to; 
      }
    }
	}
  
  void getRange(T &min, T &max) const
  {
    // Treat the array like its one dimensional to improve speed
    int tot = m_xsize * m_ysize;
    max = m_pRaster[0];
    min = m_pRaster[0];
    for( int count = 1; count < tot; count++ )
    {
      if( m_pRaster[count] > max )
      {
        max = m_pRaster[count];
      }
      else if( m_pRaster[count] < min )
      {
        min = m_pRaster[count];
      }
    }
  }

  void getRange(T &min, T &max, T ignore) const
  {
    // Treat the array like its one dimensional to improve speed
		bool bFirst = true;
    int tot = m_xsize * m_ysize;
		T val;
    for( int count = 1; count < tot; count++ )
    {
			val = m_pRaster[count];
			if( val != ignore )
			{
				if( bFirst )
				{
			    max = val;
			    min = val;
					bFirst = false;
				}
				else
				{
		      if( val > max )
  		    {
    		    max = val;
      		}
		      else if( val < min )
  		    {
	    	    min = val;
	      	}
				}
			}
    }
  }

  CHistogram *getHistogram() const
  {
    T min, max;
    getRange( min, max );
    
    CHistogram *pHist = new CHistogram(min,max+1,max-min+1);
    int tot = m_xsize * m_ysize;
    for( int count = 1; count < tot; count++ )
    {
      pHist->addValue(m_pRaster[count]);
    }
    
    return pHist;
  }
    
  CHistogram *getHistogram(T ignoreval) const
  {
    T min, max;
    getRange( min, max, ignoreval );
    
    CHistogram *pHist = new CHistogram(min,max+1,max-min+1);
    int tot = m_xsize * m_ysize;
    for( int count = 1; count < tot; count++ )
    {
      if( m_pRaster[count] != ignoreval )
      {
        pHist->addValue(m_pRaster[count]);
      }
    }
    
    return pHist;
  }

  CHistogram *getHistogram(T ignoreval, int nbins) const
  {
    T min, max;
    getRange( min, max, ignoreval );
    
    CHistogram *pHist = new CHistogram(min,max+1,nbins);
    int tot = m_xsize * m_ysize;
    for( int count = 1; count < tot; count++ )
    {
      if( m_pRaster[count] != ignoreval )
      {
        pHist->addValue(m_pRaster[count]);
      }
    }
    
    return pHist;
  }
  
  CHistogram *getHistogram(T min, T max, T ignoreval, int nbins) const
  {
    CHistogram *pHist = new CHistogram(min,max+1,nbins);
    int tot = m_xsize * m_ysize;
    for( int count = 1; count < tot; count++ )
    {
      if( m_pRaster[count] != ignoreval )
      {
        pHist->addValue(m_pRaster[count]);
      }
    }
    
    return pHist;
  }

  CHistogram *getHistogram(T min, T max, int nbins) const
  {
    CHistogram *pHist = new CHistogram(min,max+1,nbins);
    int tot = m_xsize * m_ysize;
    for( int count = 1; count < tot; count++ )
    {
        pHist->addValue(m_pRaster[count]);
    }
    
    return pHist;
  }
  
protected:
  int m_xsize;
  int m_ysize;
  T *m_pRaster;
};

// Class that grows from a point on a Raster
template<class T>
class CRegionGrow
{
public:
  CRegionGrow(CMemRaster<T> *indata,CMemRaster<T> *outdata, int minval, int maxval)
  {
    m_pIndata = indata;
    m_pOutdata = outdata;
    m_minval = minval;
    m_maxval = maxval;
  }

  void growRegion( int x, int y)
	{
	  // put the first coord in our array to check.
	  CCoordList searchList;
	  searchList.addXY(x,y);
  
	  // while we are still have pixels to search
	  while(searchList.size() != 0 )
	  {
	    // create a list with all the new coords we have found
	    CCoordList newCoords;
	    for( CCoordList::tItr itr = searchList.begin(); itr != searchList.end(); itr++)
	    {
	      CCoord coord = (*itr);
	      CCoordList tmp = searchPixel(coord.getX(),coord.getY());
	      newCoords.appendList(tmp);
	    } 
		
	    // look throught these new coords
	    searchList = newCoords;
	  }
	}
  
protected:
  CCoordList searchPixel( int x, int y )
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
	    if( x < m_pIndata->getXSize() - 1 )
	    {
  	    coordsToCheck.addXY(x-1,y+1);
	    }
	  }
	  if( x < m_pIndata->getXSize() - 1 )
	  {
	    coordsToCheck.addXY(x+1,y);
	    if( y > 0 )
	    {
	      coordsToCheck.addXY(x+1,y-1);
	    }
	    if( y < m_pIndata->getYSize() - 1 )
	    {
	      coordsToCheck.addXY(x+1,y+1);
	    }
	  }
  	if( y >  0 )
	  {
    	coordsToCheck.addXY(x,y-1);
  	}  
	  if( y < m_pIndata->getYSize() - 1 )
	  {
  	  coordsToCheck.addXY(x,y+1);
	  }
  
	  // Set the value in the output array if it is within the threshold
	  CCoordList outList;
	  for( CCoordList::tItr itr = coordsToCheck.begin(); itr != coordsToCheck.end(); itr++ )
	  {
  	  const CCoord coord = (*itr);
	    int value = m_pIndata->getValue(coord);
    	// check the value is in the range and not already set
  	  if( ( value <= m_maxval ) && ( value >= m_minval ) && ( m_pOutdata->getValue(coord) == 0 ) )
	    {
      	m_pOutdata->setValue( coord, 1 );
    	  outList.addCoord(coord);
  	  }
	  }
  
  	return outList;
	}
  CMemRaster<T> *m_pIndata;
  CMemRaster<T> *m_pOutdata;
  int m_minval;
  int m_maxval;
};


#endif //__GDALCOMMON_CPP_H___
