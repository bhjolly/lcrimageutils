/*
Implementation of methods defined in gdalcommon_cpp.h

Sam Gillingham November 2004
*/
#define _XOPEN_SOURCE 500
#define _XOPEN_SOURECE 500

#include <stdlib.h>

#include "gdal.h"
#include "cpl_string.h"


#include "gdalcommon_cpp.h"

/*===================================================================================*/

bool CCoordList::exists(const CCoord &coord) const
{
  for( tItr itr = begin(); itr != end(); itr++ )
  {
    if( (*itr).equals(coord) )
    {
      return true; 
    }
  } 
  return false;
}

bool CCoordList::addCoord(const CCoord &coord)
{
  if( !exists(coord))
  {
    push_back(coord); 
		return true;
  }
	return false;
}

bool CCoordList::appendList(const CCoordList &list)
{
  for( tItr itr = list.begin(); itr != list.end(); itr++ )
  {
    addCoord( (*itr) );    
  }
	return true;
}

// Stolen from http://www.all-science-fair-projects.com/science_fair_projects_encyclopedia/Bresenham%27s_line_algorithm_C_code
// Removes all existing points.
void CCoordList::drawline2d(const CCoord &c1, const CCoord &c2)
{
int i;
int steep = 1;
int sx, sy;  /* step positive or negative (1 or -1) */
int dx, dy;  /* delta (difference in X and Y between points) */
int e;

  int x0 = c1.getX();
  int y0 = c1.getY();
  int x1 = c2.getX();
  int y1 = c2.getY();

  clear();
	/* if we don't reserve enough ram we can get bizaree core dumps */
	reserve(64);

  /*
   * inline swap. On some architectures, the XOR trick may be faster
   */
  int tmpswap;
#define SWAP(a,b) tmpswap = a; a = b; b = tmpswap;

  /*
   * optimize for vertical and horizontal lines here
   */
   
  dx = abs(x1 - x0);
  sx = ((x1 - x0) > 0) ? 1 : -1;
  dy = abs(y1 - y0);
  sy = ((y1 - y0) > 0) ? 1 : -1;
	
  if (dy > dx) 
  {
    steep = 0;
    SWAP(x0, y0);
    SWAP(dx, dy);
    SWAP(sx, sy);
  }
  e = (dy << 1) - dx;

  for (i = 0; i < dx; i++) 
  {
    if (steep) 
    {
			push_back( CCoord(x0,y0) );
    } 
    else 
    {
			push_back( CCoord(y0,x0) );
    }
    while (e >= 0) 
    {
      y0 += sy;
      e -= (dx << 1);
    }
    x0 += sx;
    e += (dy << 1);
  }
}

/*===================================================================================*/

