#define _XOPEN_SOURCE 500
#define _XOPEN_SOURECE 500

#include <assert.h>
#include <algorithm>
#include <set>

#include "regionlabelling.h"


// Helper classes for label().
// we store all the equiv vales as a set<> to avoid duplication
typedef std::set<int> tIntSet;
typedef tIntSet::iterator tIntSetItr;

// Manages addition to equivs as per comments in addEquiv()
// Is a vector of sets. Each id in a set is equivalent.
class CEquivalentIDS: public std::vector<tIntSet>
{
public: 
  typedef std::vector<tIntSet>::iterator tItr;

  // Adds an equivalence to our list of equivalences
  void addEquiv(int id1,int id2)
  {
    tItr itrID1 = findID(id1);
    tItr itrID2 = findID(id2);
    
    // 3 cases:
    if( ( itrID1 == end() ) && ( itrID2 == end() ) )
    {
      // 1. We haven't seen this equiv before - put it in its own new set 
      tIntSet newEquiv;
      newEquiv.insert(id1);
      newEquiv.insert(id2);
      push_back(newEquiv);
    }
    else if( ( itrID1 != itrID2 ) && ( itrID1 != end() ) && ( itrID2 != end() ) )
    {
      // 2. the ids are in different sets - merge them
      tIntSet *set1 = &(*itrID1);
      tIntSet *set2 = &(*itrID2);
      
      set1->insert(set2->begin(),set2->end());
      erase(itrID2);
    }
    else if( itrID1 != itrID2 )
    {
      // 3. One is in our list - the other isn't
      if( itrID1 != end() )
      {
        tIntSet *set1 = &(*itrID1);
        set1->insert(id2);
      }
      else
      {
        tIntSet *set2 = &(*itrID2);
        set2->insert(id1);
      }
    }
    // If they are in the same set anyway, just return.
  }
  
  tItr findID(int id)
  {
    for( tItr itr = begin(); itr != end(); itr++ )
    {
      tIntSet *set = &(*itr);
      if( find(set->begin(),set->end(),id) != set->end() )
      {
        return itr;
      }
    }
    return end();
  }
  
};

// Labels contiguous non-zero pixels as unique regions. Output layer in region IDS.
// See http://www.stanford.edu/class/ee368/Handouts/3-RegionLabelingCounting.pdf
// Returns the highest label assigned.
// If bCompact set to true the values are recoded so they are contguous which is a bit slower.
int CRegionLabelling::label(CMemRaster<GByte> &inoutData, bool bCompact)
{
  int nLastID = 0;
  
  // Our unique id counter
  int nIDValue = 1;
  
  // List of unique values
  CEquivalentIDS equivIDS;

  // Go through each pixel
  for( int y = 0; y < inoutData.getYSize(); y++ )
  {
    for( int x = 0; x < inoutData.getXSize(); x++ )
    {
      if( inoutData.getValue(x,y) != 0 )
      {
        // OK have found a new pixel. Distinguish 4 cases
        int nLeft = 0;
        if( x > 0 )
        {
          nLeft = inoutData.getValue(x - 1,y);
        }
        int nUp = 0;
        if( y > 0 )
        {
          nUp = inoutData.getValue(x,y - 1);
        }
        
        if( ( nLeft == 0 ) && ( nUp == 0 ) )
        {
          // generate new label
          inoutData.setValue(x,y,nIDValue);
          nIDValue++; 
        }
        else if( ( nLeft == 0 ) && ( nUp != 0 ) )
        {
          // copy label from above
          inoutData.setValue(x,y,nUp);
        }
        else if( ( nLeft != 0 ) && ( nUp == 0 ) )
        {
          // copy label from the left
          inoutData.setValue(x,y,nLeft);
        }
        else
        {
          // both the left and above are non zero
          // Copy the label from the left
          inoutData.setValue(x,y,nLeft);
          
          if( nLeft != nUp )
          {
            // if above and left different, store equivalence
            equivIDS.addEquiv( nLeft,nUp );
          }
        }
      }
    }
  }
  
  // Recode the values as per the equivs we have recorded.
  for( CEquivalentIDS::tItr itr = equivIDS.begin(); itr != equivIDS.end(); itr++ )
  {
     tIntSet *set = &(*itr);
     int minval = (*min_element(set->begin(),set->end()));
     int val;
     for( tIntSetItr nitr = set->begin(); nitr != set->end(); nitr++ )
     {
        val = (*nitr);
        if( minval != val )
        {
          inoutData.recode(val,minval);
        }
     }
  }
  
  // Compact numbers if necessary
  if( bCompact )
  {
    // Get the histogram and use it to compact
    CHistogram *pHisto = inoutData.getHistogram();
    
    int fillfrombin = pHisto->getNBins() - 1;
    int count = 0;
    while( count < fillfrombin )
    {
      int bincount = pHisto->getBinCount(count);
      if( bincount == 0 )
      {
        // need to fill in this value with something at the end
        while( pHisto->getBinCount(fillfrombin) == 0 )
        {
          fillfrombin--;
        }

        GByte from = (GByte)pHisto->getValueForBin(fillfrombin);
        GByte to = (GByte)pHisto->getValueForBin(count);
        //printf( "recoding %d to %d\n", (int)from, (int)to);
        inoutData.recode(from,to);
        fillfrombin--;
      }
      count++;
    }
  
    nLastID = count - 1;
    delete pHisto;
  }
  else
  {
    // Can be wrong if there is a recode??
    nLastID = nIDValue - 1;
  }
  
  return nLastID;
}
