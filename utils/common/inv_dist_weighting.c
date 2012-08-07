
#include <math.h>
#include <stdlib.h>

/*extern "C"
{*/
	#include "inv_dist_weighting.h"
/*}*/

float idw_findDist( SIDWInfo *pInfo, float x2, float y2 )
{
float fXDist, fYDist;

  fXDist = pInfo->x - x2;
  fYDist = pInfo->y - y2;
  return sqrtf( fXDist * fXDist + fYDist * fYDist );
}

/* Finds the weights to the points supplied */
float* idw_findWeights( SIDWInfo *pInfo, int numpoints, float pointx, float pointy, float N )
{
float *pWeights, *pDistances;
int count;

  /* Work out the distances to each point */
  pDistances = (float*)calloc( numpoints, sizeof( float ) );
  
  for( count = 0; count < numpoints; count++ )
  {
    pDistances[count] = idw_findDist( &pInfo[count], pointx, pointy );
  }

  pWeights = idw_findWeights_with_dist( pDistances, numpoints, N );
  
  free( pDistances );
  
  return pWeights;
}

/* Goven distances already worked out find the weights */
float* idw_findWeights_with_dist( float *dists, int numpoints, float N )
{
float *pWeights, ftotalDist = 0, ftotalWeight = 0;  
int count;

  /* find the total distance */
  for( count = 0; count < numpoints; count++ )
  {
    ftotalDist += dists[count];
  }
  
  /* Work out the weights as 1 over the fraction of the total distance */
  pWeights = (float*)calloc( numpoints, sizeof( float ) );
  for( count = 0; count < numpoints; count++ )
  {
    pWeights[count] = 1 / powf( dists[count] / ftotalDist, N );
    ftotalWeight += pWeights[count];
  }
  
  /* rescale the weights so that they all add to one */
  for( count = 0; count < numpoints; count++ )
  {
    pWeights[count] = pWeights[count] / ftotalWeight;
  }

  return pWeights;
}


float idw_applyWeights( float *weights, float *values, int numpoints )
{
float val = 0;
int count; 
  
  for( count = 0; count < numpoints; count++ )
  {
    val += weights[count] * values[count];
  }
  
  return val;
}
